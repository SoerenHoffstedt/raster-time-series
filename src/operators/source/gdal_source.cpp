
#include "datatypes/raster_operations.h"
#include "operators/source/gdal_source.h"
#include "util/raster_calculations.h"
#include "util/parsing.h"
#include <filesystem>
#include <fstream>
#include <ctime>
#include <gdal.h>

using namespace rts;
using namespace boost::posix_time;

template<class T>
struct GdalSourceWriter {
    static void rasterOperation(TypedRaster<T> *raster, std::shared_ptr<GDALDataset> rasterDataset,
                                GDALRasterBand *rasterBand, const Descriptor &self,
                                Resolution fill_from, Resolution res_left_to_fill)
    {
        Resolution tileRes = raster->getResolution();

        //calculate where the qrect is in source raster pixels. see mappings gdalsource.
        double adfGeoTransform[6];
        if(rasterDataset->GetGeoTransform( adfGeoTransform ) != CE_None ) {
            throw std::runtime_error("GDAL Source: No GeoTransform information in raster");
        }

        double origin_x = adfGeoTransform[0];
        double origin_y = adfGeoTransform[3];
        double scale_x = adfGeoTransform[1];
        double scale_y = adfGeoTransform[5];

        //TODO: why is the origin swapped here? make this hack go away. (in mapping i also had to flip the loaded raster hor.)
        if(origin_y == 90.0){
            auto extent = self.rasterInfo.projection.getExtent();
            if(extent.y1 == -90.0){
                origin_y = extent.y1;
                scale_y *= -1;
            }
        }

        SpatialReference spatInfo = self.tileSpatialInfo;
        if(spatInfo.x1 < self.rasterInfo.x1)
            spatInfo.x1 = self.rasterInfo.x1;
        if(spatInfo.x2 > self.rasterInfo.x2)
            spatInfo.x2 = self.rasterInfo.x2;
        if(spatInfo.y1 < self.rasterInfo.y1)
            spatInfo.y1 = self.rasterInfo.y1;
        if(spatInfo.y2 > self.rasterInfo.y2)
            spatInfo.y2 = self.rasterInfo.y2;

        int rasterSizeX = rasterBand->GetXSize();
        int rasterSizeY = rasterBand->GetYSize();

        int pixel_x1 = 0;
        int pixel_y1 = 0;
        int pixel_x2 = rasterSizeX;
        int pixel_y2 = rasterSizeY;
        int pixel_width = rasterSizeX;
        int pixel_height = rasterSizeY;

        pixel_x1 = static_cast<int>(floor((spatInfo.x1 - origin_x) / scale_x));
        pixel_y1 = static_cast<int>(floor((spatInfo.y1 - origin_y) / scale_y));
        pixel_x2 = static_cast<int>(floor((spatInfo.x2 - origin_x) / scale_x));
        pixel_y2 = static_cast<int>(floor((spatInfo.y2 - origin_y) / scale_y));

        if (pixel_x1 > pixel_x2)
            std::swap(pixel_x1, pixel_x2);
        if (pixel_y1 > pixel_y2)
            std::swap(pixel_y1, pixel_y2);

        pixel_width = pixel_x2 - pixel_x1; // + 1;
        pixel_height = pixel_y2 - pixel_y1; // + 1;

        int gdal_pixel_x1 = std::min(rasterSizeX - 1, std::max(0, pixel_x1));
        int gdal_pixel_y1 = std::min(rasterSizeY - 1, std::max(0, pixel_y1));

        int gdal_pixel_x2 = std::min(rasterSizeX - 1, std::max(0, pixel_x2));
        int gdal_pixel_y2 = std::min(rasterSizeY - 1, std::max(0, pixel_y2));

        int gdal_pixel_width = gdal_pixel_x2 - gdal_pixel_x1;// + 1;
        int gdal_pixel_height = gdal_pixel_y2 - gdal_pixel_y1;// + 1;

        Resolution size = tileRes - fill_from;
        if(res_left_to_fill.res_x < size.res_x)
            size.res_x -= res_left_to_fill.res_x;
        if(res_left_to_fill.res_y < size.res_y)
            size.res_y -= res_left_to_fill.res_y;

        if(fill_from.res_x > 0 || fill_from.res_y > 0 || size.res_x < tileRes.res_x || size.res_y < tileRes.res_y){
            for (int x = 0; x < tileRes.res_x; ++x) {
                for (int y = 0; y < tileRes.res_y; ++y) {
                    raster->setCell(x,y, (T)self.nodata);
                }
            }
        }

        void *buffer = nullptr;
        if(fill_from.res_x > 0 || fill_from.res_y > 0)
            buffer = raster->getVoidDataPointerOffset(fill_from.res_x, fill_from.res_y);
        else
            buffer = raster->getVoidDataPointer();

        auto res = rasterBand->RasterIO(GF_Read, gdal_pixel_x1, gdal_pixel_y1, gdal_pixel_width, gdal_pixel_height,
                buffer, size.res_x, size.res_y, self.dataType, 0, sizeof(T) * tileRes.res_x, nullptr);

        if(res != CE_None){
            throw std::runtime_error("GDAL Source: Reading from raster failed.");
        }
    }
};

GDALSource::GDALSource(const QueryRectangle &qrect, Json::Value &params, UniqueOperatorVector &&in)
        : GenericOperator(qrect, params, std::move(in)), currDataset(nullptr), currRasterband(nullptr), currTileIndex(0), currRasterIndex(0)
{
    GDALUtil::initGdal();

    Json::Value dataset_json    = loadDatasetJson(params["dataset"].asString());
    tileRes                     = Resolution(params["tile_size_x"].asUInt(), params["tile_size_y"].asUInt());
    channel                     = dataset_json["channel"].asInt();

    file_name_base              = dataset_json["filename"].asString();
    path                        = dataset_json["path"].asString();
    time_format                 = dataset_json["time_format"].asString();

    Json::Value time_interval_json   = dataset_json["time_interval"];
    time_interval = TimeInterval(time_interval_json);

    dataset_time_start          = time_from_string(dataset_json["time_start"].asString());
    dataset_time_end            = time_from_string(dataset_json["time_end"].asString());
    curr_time                   = dataset_time_start;

    //advance start point for raster, until it is not smaller than t1.
    while(to_time_t(curr_time) < qrect.t1 && getCurrentTimeEnd() < qrect.t1){
        increaseCurrentTime();
    }

    //calc number of tiles
    rasterWorldPixelStart = RasterCalculations::coordinateToWorldPixel(qrect, qrect.x1, qrect.y1);

    Resolution rasterStep = rasterWorldPixelStart;
    rasterStep.res_x -= rasterWorldPixelStart.res_x % tileRes.res_x;
    rasterStep.res_y -= rasterWorldPixelStart.res_y % tileRes.res_y;
    Resolution rasterWorldPixelEnd = RasterCalculations::coordinateToWorldPixel(qrect, qrect.x2, qrect.y2);
    Resolution size(rasterWorldPixelEnd.res_x - rasterStep.res_x, rasterWorldPixelEnd.res_y - rasterStep.res_y);
    uint32_t num_x = size.res_x / tileRes.res_x;
    uint32_t num_y = size.res_y / tileRes.res_y;
    if(size.res_x % tileRes.res_x > 0)
        num_x += 1;
    if(size.res_y % tileRes.res_y > 0)
        num_y += 1;
    tileCount = num_x * num_y;
}

OptionalDescriptor GDALSource::nextDescriptor() {

    time_t curr_time_t = to_time_t(curr_time);

    if(curr_time_t > qrect.t2 || curr_time > dataset_time_end){
        return std::nullopt;
    }

    if(currTileIndex >= tileCount){
        return std::nullopt;
    }

    if(currDataset == nullptr){
        loadCurrentGdalDataset();
    }

    double nodata = currRasterband->GetNoDataValue();
    GDALDataType dataType = currRasterband->GetRasterDataType();

    Resolution fill_from(0, 0);

    //fill_from: for fixed alignment of tiles, start of a tile is not always 0, based on what pixel in world space the tile starts.
    if(state_x == 0) {
        fill_from.res_x = rasterWorldPixelStart.res_x % tileRes.res_x;
        state_x -= fill_from.res_x; //state_x will have tile_res.res_x added later on, so align it to the tiles here.
    } else if(state_x < 0){
        fill_from.res_x = rasterWorldPixelStart.res_x % tileRes.res_x;
    }
    if(state_y == 0) {
        fill_from.res_y = rasterWorldPixelStart.res_y % tileRes.res_y;
        state_y -= fill_from.res_y;
    } else if(state_y < 0){
        fill_from.res_y = rasterWorldPixelStart.res_y % tileRes.res_y;
    }
    //total pixel left to fill, may be bigger as tileSize
    Resolution res_left_to_fill(qrect.res_x - state_x, qrect.res_y - state_y);

    Resolution tile_start_world_res(rasterWorldPixelStart.res_x + state_x, rasterWorldPixelStart.res_y + state_y);
    SpatialReference tile_spat = RasterCalculations::calcSpatialInfoFromPixel(qrect, tile_start_world_res,
                                                                              tile_start_world_res + tileRes);

    auto getter = [currDataset = currDataset, currRasterband = currRasterband, fill_from = fill_from, res_left_to_fill = res_left_to_fill](const Descriptor &self) -> std::unique_ptr<Raster> {
        std::unique_ptr<Raster> out = Raster::createRaster(self.dataType, self.tileResolution);
        RasterOperations::callUnary<GdalSourceWriter>(out.get(), currDataset, currRasterband, self, fill_from, res_left_to_fill);
        return out;
    };

    TemporalReference tempInfo(static_cast<double>(to_time_t(curr_time)), getCurrentTimeEnd());
    int tileIndexNow = currTileIndex;
    SpatialTemporalReference rasterInfo = qrect;
    rasterInfo.t1 = tempInfo.t1;
    rasterInfo.t2 = tempInfo.t2;

    if(qrect.order == Order::Temporal){
        if(increaseSpatial()){
            increaseTemporal();
        }
    } else if(qrect.order == Order::Spatial){
        if(increaseTemporal()){
            increaseSpatial();
        }
    }

    return std::make_optional<Descriptor>(std::move(getter), rasterInfo, tile_spat, tileRes,
                                          qrect.order, tileIndexNow, tileCount, nodata, dataType);

}

bool GDALSource::increaseTemporal() {
    currRasterIndex += 1;
    currDataset = nullptr;
    increaseCurrentTime();
    auto curr_time_t = static_cast<double>(to_time_t(curr_time));
    if(curr_time_t > qrect.t2 || curr_time > dataset_time_end){
        if(qrect.order == Order::Spatial){
            //reset only if order is spatial, else keep time_curr above t2/dataset_end_time as end condition at top of nextDescriptor() method
            currRasterIndex = 0;
            curr_time = dataset_time_start;
            while(to_time_t(curr_time) < qrect.t1){
                increaseCurrentTime();
            }
            return true;
        }
    }
    return false;
}

bool GDALSource::increaseSpatial() {

    currTileIndex += 1;
    state_x += tileRes.res_x;
    if(state_x >= static_cast<int>(qrect.res_x)){
        state_x = 0;
        state_y += tileRes.res_y;
        if(state_y >= static_cast<int>(qrect.res_y)){
            if(qrect.order == Order::Temporal){
                //reset only if spatial is the outer dimension, else keep state_y as end condition at start of nextDescriptor() method.
                currTileIndex = 0;
                state_y = 0;
                return true;
            }
        }
    }
    return false;
}

bool GDALSource::supportsOrder(Order o) const {
    return o == Order::Temporal || o == Order::Spatial;
}

Json::Value GDALSource::loadDatasetJson(const std::string &name) {
    std::filesystem::path p("../../test/data/gdal_source");
    p /= std::filesystem::path(name + ".json");
    std::ifstream file_in(p.string());
    Json::Value dataset_json;
    file_in >> dataset_json;
    return dataset_json;
}

double GDALSource::parseIsoTime(const std::string &str) const {
    std::tm tm = {};
    if (strptime(str.c_str(), "%Y-%m-%dT%H:%M:%S", &tm))
        return (double)timegm(&tm);
    else
        throw std::runtime_error("Could not parse time.");
}

void GDALSource::increaseCurrentTime() {
    time_interval.increase(curr_time);
}

double GDALSource::getCurrentTimeEnd() const {
    ptime curr = curr_time;
    time_interval.increase(curr);
    return static_cast<double>(to_time_t(curr));
}

constexpr int MAX_FILE_NAME_LENGTH = 255;

void GDALSource::loadCurrentGdalDataset() {
    
    char date[MAX_FILE_NAME_LENGTH] = {0};
    time_t curr_time_t = to_time_t(curr_time);
    tm curr_time_tm = *gmtime(&curr_time_t);
    strftime(date, sizeof(date), time_format.c_str(), &curr_time_tm);
    std::string timeString(date);

    std::string placeholder = "%%%TIME_STRING%%%";
    size_t placeholderPos = file_name_base.find(placeholder);

    std::string fileName = file_name_base;
    fileName.replace(placeholderPos, placeholder.length(), timeString);

    if(openDatasets.find(fileName) != openDatasets.end())
    {
        currDataset = openDatasets[fileName];
        currRasterband = currDataset->GetRasterBand(channel);
    }
    else
    {
        std::filesystem::path file_path(path.c_str());
        file_path /= fileName + ""; //i dont get how you can not append a std::filesystem::path with a std::string ?!?!

        auto dataset = (GDALDataset *)GDALOpen(file_path.c_str(), GA_ReadOnly);
        if(dataset == nullptr){
            throw std::runtime_error("GDAL dataset could not be opened: " + file_path.string());
        }
        currRasterband = dataset->GetRasterBand(channel);

        currDataset = std::shared_ptr<GDALDataset>(dataset, [](GDALDataset *d){
            GDALClose(d);
            d = nullptr;
        });
        //cache the dataset when its spatial order.
        if(qrect.order == Order::Spatial)
            openDatasets[fileName] = currDataset;
    }

}
