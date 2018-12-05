
#include "datatypes/raster_operations.h"
#include "operators/source/gdal_source.h"
#include "util/raster_calculations.h"
#include "util/parsing.h"
#include <filesystem>
#include <fstream>
#include <ctime>

using namespace rts;
using namespace boost::posix_time;

template<class T>
struct GdalSourceWriter {
    static void rasterOperation(TypedRaster<T> *raster, std::shared_ptr<GDALDataset> rasterDataset, GDALRasterBand *rasterBand, const Descriptor &self) {
        Resolution tileRes = raster->getResolution();
        void *buffer = static_cast<void*>(raster->getDataPointer());

        //calculate where the qrect is in source raster pixels. see mappings gdalsource.

        double adfGeoTransform[6];
        if(rasterDataset->GetGeoTransform( adfGeoTransform ) != CE_None ) {
            throw std::runtime_error("GDAL Source: No GeoTransform information in raster");
        }

        double origin_x = adfGeoTransform[0];
        double origin_y = adfGeoTransform[3];
        double scale_x = adfGeoTransform[1];
        double scale_y = adfGeoTransform[5];

        double clip_x1 = self.tileSpatialInfo.x1;
        double clip_y1 = self.tileSpatialInfo.y1;
        double clip_x2 = self.tileSpatialInfo.x2;
        double clip_y2 = self.tileSpatialInfo.y2;

        int nXSize = rasterBand->GetXSize();
        int nYSize = rasterBand->GetYSize();

        int pixel_x1 = 0;
        int pixel_y1 = 0;
        int pixel_x2 = nXSize - 1;
        int pixel_y2 = nYSize - 1;
        int pixel_width = nXSize;
        int pixel_height = nYSize;
        bool clip = true;
        if (clip) {
            pixel_x1 = static_cast<int>(floor((clip_x1 - origin_x) / scale_x));
            pixel_y1 = static_cast<int>(floor((clip_y1 - origin_y) / scale_y));
            pixel_x2 = static_cast<int>(floor((clip_x2 - origin_x) / scale_x));
            pixel_y2 = static_cast<int>(floor((clip_y2 - origin_y) / scale_y));

            if (pixel_x1 > pixel_x2)
                std::swap(pixel_x1, pixel_x2);
            if (pixel_y1 > pixel_y2)
                std::swap(pixel_y1, pixel_y2);

            pixel_width = pixel_x2 - pixel_x1 + 1;
            pixel_height = pixel_y2 - pixel_y1 + 1;
        }

        int gdal_pixel_x1 = std::min(nXSize - 1, std::max(0, pixel_x1));
        int gdal_pixel_y1 = std::min(nYSize - 1, std::max(0, pixel_y1));

        int gdal_pixel_x2 = std::min(nXSize - 1, std::max(0, pixel_x2));
        int gdal_pixel_y2 = std::min(nYSize - 1, std::max(0, pixel_y2));

        int gdal_pixel_width = gdal_pixel_x2 - gdal_pixel_x1 + 1;
        int gdal_pixel_height = gdal_pixel_y2 - gdal_pixel_y1 + 1;

        rasterBand->RasterIO(GF_Read, gdal_pixel_x1, gdal_pixel_y1, gdal_pixel_width, gdal_pixel_height,
                             buffer, tileRes.res_x, tileRes.res_y, self.dataType, 0, 0, nullptr);

        int xsizenow = rasterBand->GetXSize();
        int ysizenow = rasterBand->GetYSize();
        int bla = 10;
    }
};

GDALSource::GDALSource(const QueryRectangle &qrect, Json::Value &params, UniqueOperatorVector &&in)
        : GenericOperator(qrect, params, std::move(in)), currDataset(nullptr), currRasterband(nullptr), currTileIndex(0), currRasterIndex(0)
{
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
    while(to_time_t(curr_time) < qrect.t1){
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

    Resolution tile_start_world_res(rasterWorldPixelStart.res_x + state_x, rasterWorldPixelStart.res_y + state_y);
    SpatialReference tile_spat = RasterCalculations::calcSpatialInfoFromTilePixel(qrect, tile_start_world_res, tile_start_world_res + tileRes);

    // not checking fill_from, left_to_fill like in FakeSource, because if the data exists in the file, just load it.
    // alternatively add later to only load the data really needed.
    auto getter = [currDataset = currDataset, currRasterband = currRasterband](const Descriptor &self) -> std::unique_ptr<Raster> {
        std::unique_ptr<Raster> out = Raster::createRaster(self.dataType, self.tileResolution);
        RasterOperations::callUnary<GdalSourceWriter>(out.get(), currDataset, currRasterband, self);
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
    //TODO:: change dataset, etc if needed.
    currRasterIndex += 1;
    increaseCurrentTime();
    auto curr_time_t = static_cast<double>(to_time_t(curr_time));
    if(curr_time_t > qrect.t2){
        if(qrect.order == Order::Spatial){
            //reset only if temporal is the outer dimension, else keep time_curr above t2 as end condition at top of nextDescriptor() method
            currRasterIndex = 0;
            curr_time = dataset_time_start;
            while(to_time_t(curr_time) < qrect.t1){
                increaseCurrentTime();
            }
            loadCurrentGdalDataset();
            return true;
        }
    }
    loadCurrentGdalDataset();
    return false;
}

bool GDALSource::increaseSpatial() {
    //TODO:: change dataset, etc if needed.
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

bool GDALSource::supportsOrder(Order o) {
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

    GDALUtil::initGdal();
    
    char date[MAX_FILE_NAME_LENGTH] = {0};
    time_t curr_time_t = to_time_t(curr_time);
    tm curr_time_tm = *gmtime(&curr_time_t);
    strftime(date, sizeof(date), time_format.c_str(), &curr_time_tm);
    std::string timeString(date);

    std::string placeholder = "%%%TIME_STRING%%%";
    size_t placeholderPos = file_name_base.find(placeholder);

    std::string fileName = file_name_base;
    fileName.replace(placeholderPos, placeholder.length(), timeString);

    std::filesystem::path file_path(path.c_str());
    file_path /= fileName + ""; //i dont get how you can not append a std::filesystem::path with a std::string ?!?!

    GDALDataset *dataset = (GDALDataset *)GDALOpen(file_path.c_str(), GA_ReadOnly);
    if(dataset == nullptr){
        throw std::runtime_error("GDAL dataset could not be opened: " + file_path.string());
    }
    currRasterband = dataset->GetRasterBand(channel);

    currDataset = std::shared_ptr<GDALDataset>(dataset, [](GDALDataset *d){
        GDALClose(d);
        d = nullptr;
    });

}
