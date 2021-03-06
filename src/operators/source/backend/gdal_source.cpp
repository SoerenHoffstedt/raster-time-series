
#include "datatypes/raster_operations.h"
#include "util/raster_calculations.h"
#include "util/parsing.h"
#include "util/benchmark.h"
#include "gdal_source.h"

#include <boost/filesystem.hpp>
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

        int rasterSizeX = rasterBand->GetXSize();
        int rasterSizeY = rasterBand->GetYSize();

        //GDAL often has a positive y origin and a negative scale, but I assume the origin to be the smaller
        //coordinate and the scale to be positive, so swap that here. use raster size for calculation of actual origin.
        if(scale_y < 0){
            double old_origin_y = origin_y;
            origin_y = origin_y + scale_y * rasterSizeY;
            scale_y *= -1;
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

        pixel_width = pixel_x2 - pixel_x1;
        pixel_height = pixel_y2 - pixel_y1;

        int gdal_pixel_x1 = std::min(rasterSizeX, std::max(0, pixel_x1));
        int gdal_pixel_y1 = std::min(rasterSizeY, std::max(0, pixel_y1));

        int gdal_pixel_x2 = std::min(rasterSizeX, std::max(0, pixel_x2));
        int gdal_pixel_y2 = std::min(rasterSizeY, std::max(0, pixel_y2));

        int gdal_pixel_width = gdal_pixel_x2 - gdal_pixel_x1;
        int gdal_pixel_height = gdal_pixel_y2 - gdal_pixel_y1;

        Resolution size = tileRes - fill_from;
        if(res_left_to_fill.resX < size.resX)
            size.resX = res_left_to_fill.resX - fill_from.resX;
        if(res_left_to_fill.resY < size.resY)
            size.resY = res_left_to_fill.resY - fill_from.resY;

        if(fill_from.resX > 0 || fill_from.resY > 0 || size.resX < tileRes.resX || size.resY < tileRes.resY){
            for (int x = 0; x < tileRes.resX; ++x) {
                for (int y = 0; y < tileRes.resY; ++y) {
                    raster->setCell(x,y, (T)self.nodata);
                }
            }
        }

        void *buffer = nullptr;
        if(fill_from.resX > 0 || fill_from.resY > 0)
            buffer = raster->getVoidDataPointerOffset(fill_from.resX, fill_from.resY);
        else
            buffer = raster->getVoidDataPointer();

        auto res = rasterBand->RasterIO(GF_Read, gdal_pixel_x1, gdal_pixel_y1, gdal_pixel_width, gdal_pixel_height,
                buffer, size.resX, size.resY, self.dataType, 0, sizeof(T) * tileRes.resX, nullptr);

        if(res != CE_None){
            throw std::runtime_error("GDAL Source: Reading from raster failed.");
        }
    }
};


GDALSource::GDALSource(const QueryRectangle &qrect, const Json::Value &params)
        : SourceBackend(qrect, params), currDataset(nullptr), currRasterband(nullptr), currDatasetTime(0)
{

}

void GDALSource::initialize() {
    GDALUtil::initGdal();

    Json::Value dataset_json    = loadDatasetJson(params["dataset"].asString());
    channel                     = dataset_json["channel"].asInt();

    baseFileName                = dataset_json["filename"].asString();
    path                        = dataset_json["path"].asString();
    timeFormat                  = dataset_json["time_format"].asString();

    Json::Value time_interval_json   = dataset_json["time_interval"];
    timeInterval = TimeInterval(time_interval_json);

    datasetStartTime = (double)to_time_t(time_from_string(dataset_json["time_start"].asString()));
    datasetEndTime   = (double)to_time_t(time_from_string(dataset_json["time_end"].asString()));

    Json::Value coords = dataset_json["coords"];

    if(params["dataset"] == "msg_eu_scaled_int") {
        //TODO: bad hack for meteosat queries. test properly what the error is.
        auto extent = SpatialReference(coords["extent"]);
        origin.x = extent.x1;
        origin.y = extent.y1;
    } else {
        origin = qrect.projection.getOrigin();
    }

}

OptionalDescriptor GDALSource::createDescriptor(double time, int pixelStartX, int pixelStartY, int tileIndex, const Resolution &rasterWorldPixelStart, const Scale &scale, const Origin &origin, const Resolution &tileCount) {

    if(currDataset == nullptr || time != currDatasetTime){
        loadCurrentGdalDataset(time);
    }

    double nodata = currRasterband->GetNoDataValue();
    GDALDataType dataType = currRasterband->GetRasterDataType();

    Resolution fillFrom(0, 0);

    //fillFrom: for fixed alignment of tiles, start of a tile is not always 0, based on what pixel in world space the tile starts.
    if(pixelStartX < 0){
        fillFrom.resX = (uint32_t)(-1 * pixelStartX);
    }
    if(pixelStartY < 0){
        fillFrom.resY = (uint32_t)(-1 * pixelStartY);
    }

    //total pixel left to fill, may be bigger as tileSize
    Resolution resLeftToFill(qrect.resX - pixelStartX, qrect.resY - pixelStartY); //pixelInTileLeftToFill

    Resolution tileStartWorldRes(rasterWorldPixelStart.resX + pixelStartX, rasterWorldPixelStart.resY + pixelStartY);
    SpatialReference tileSpat = RasterCalculations::pixelToSpatialRectangle(scale, origin, tileStartWorldRes, tileStartWorldRes + qrect.tileRes);

    auto getter = [currDataset = currDataset, currRasterband = currRasterband, fillFrom = fillFrom, resLeftToFill = resLeftToFill](const Descriptor &self) -> std::unique_ptr<Raster> {
        Benchmark::startSource();
        std::unique_ptr<Raster> out = Raster::createRaster(self.dataType, self.tileResolution);
        RasterOperations::callUnary<GdalSourceWriter>(out.get(), currDataset, currRasterband, self, fillFrom, resLeftToFill);
        Benchmark::endSource();
        return out;
    };

    TemporalReference tempInfo(time, getCurrentTimeEnd(time));
    SpatialTemporalReference rasterInfo = qrect;
    rasterInfo.t1 = tempInfo.t1;
    rasterInfo.t2 = tempInfo.t2;

    return rts::make_optional<Descriptor>(std::move(getter), rasterInfo, tileSpat, qrect.tileRes,
                                          qrect.order, tileIndex, tileCount, nodata, dataType);
}

Origin GDALSource::getOrigin() const {
    return origin;
}

void GDALSource::beforeTemporalIncrease(){
    currDataset = nullptr;
}

bool GDALSource::supportsOrder(Order o) const {
    return o == Order::Temporal || o == Order::Spatial;
}

void GDALSource::increaseCurrentTime(double &currTime) {
    ptime currPTime = from_time_t((time_t)currTime);
    timeInterval.increase(currPTime);
    currTime = (double)to_time_t(currPTime);
}

double GDALSource::getCurrentTimeEnd(double currTime) const {
    ptime currPTime = from_time_t((time_t)currTime);
    timeInterval.increase(currPTime);
    return static_cast<double>(to_time_t(currPTime));
}

Json::Value GDALSource::loadDatasetJson(const std::string &name) {
    boost::filesystem::path p("../../test/data/gdal_source");
    p /= name + ".json";
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

void GDALSource::loadCurrentGdalDataset(double time) {
    currDatasetTime = time;
    std::string timeString = GDALUtil::timeToString((time_t)time, timeFormat);

    std::string placeholder = "%%%TIME_STRING%%%";
    size_t placeholderPos = baseFileName.find(placeholder);

    std::string fileName = baseFileName;
    fileName.replace(placeholderPos, placeholder.length(), timeString);

    if(openDatasets.find(fileName) != openDatasets.end())
    {
        currDataset = openDatasets[fileName];
        currRasterband = currDataset->GetRasterBand(channel);
    }
    else
    {
        boost::filesystem::path file_path(path.c_str());
        file_path /= fileName; //i dont get how you can not append a std::filesystem::path with a std::string ?!?!

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
