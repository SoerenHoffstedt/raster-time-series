
#include <fstream>
#include <iostream>
#include <string>
#include <functional>
#include <boost/filesystem.hpp>
#include "datatypes/raster_operations.h"
#include "datatypes/descriptor.h"
#include "util/raster_calculations.h"
#include "util/parsing.h"
#include "fake_source.h"

using namespace rts;

template<class T>
struct FakeSourceWriter {
    static void rasterOperation(TypedRaster<T> *raster, Resolution fill_from, Resolution res_left_to_fill, int index, double nodata, bool fillIndex){
        Resolution res = raster->getResolution();
        for (int x = 0; x < res.resX; ++x) {
            for (int y = 0; y < res.resY; ++y) {
                int val = fillIndex ? index : x + y;
                if(x >= fill_from.resX && y >= fill_from.resY && x < res_left_to_fill.resX && y < res_left_to_fill.resY)
                    raster->setCell(x, y, (T)val);
                else
                    raster->setCell(x, y, (T)nodata);
            }
        }
    }
};

FakeSource::FakeSource(const QueryRectangle &qrect, const Json::Value &params) : SourceBackend(qrect, params)
{
    lastTime = -1;
    currRasterIndex = 0;
}

void FakeSource::initialize() {
    auto dataset_json = loadDatasetJson(params["dataset"].asString());
    rasterCount = dataset_json["raster_count"].asInt();
    datasetStartTime = dataset_json["time_start"].asDouble();
    datasetEndTime = std::numeric_limits<double>::max();
    timeDuration = dataset_json["time_duration"].asDouble();

    nodata = dataset_json["nodata"].asDouble();
    dataType = Parsing::parseDataType(dataset_json["data_type"].asString());

    extent = qrect.projection.getExtent();
    fill_with_index = params.get("fill_with_index", false).asBool();
}

Json::Value FakeSource::loadDatasetJson(std::string name) {
    boost::filesystem::path p("../../test/data/fake_source");
    p /= name + ".json";
    std::ifstream file_in(p.string());
    Json::Value dataset_json;
    file_in >> dataset_json;
    return dataset_json;
}

OptionalDescriptor FakeSource::createDescriptor(double time, int pixelStartX, int pixelStartY, int tileIndex, const Resolution &rasterWorldPixelStart, const Scale &scale, const Origin &origin, const Resolution &tileCount) {

    Resolution fillFrom(0, 0);

    //fill_from: for fixed alignment of tiles, start of a tile is not always 0, based on what pixel in world space the tile starts.
    if(pixelStartX < 0){
        fillFrom.resX = (uint32_t)(-1 * pixelStartX);
    }
    if(pixelStartY < 0){
        fillFrom.resY = (uint32_t)(-1 * pixelStartY);
    }
    Resolution res_left_to_fill(qrect.resX - pixelStartX, qrect.resY - pixelStartY);

    Resolution tile_start_world_res(rasterWorldPixelStart.resX + pixelStartX, rasterWorldPixelStart.resY + pixelStartY);
    SpatialReference tile_spat = RasterCalculations::pixelToSpatialRectangle(qrect, tile_start_world_res, tile_start_world_res + qrect.tileRes);

    if(lastTime > -1 && lastTime < time){
        currRasterIndex += 1;
        lastTime = time;
    }

    auto getter = [index = currRasterIndex, res_left_to_fill = res_left_to_fill, fillFrom = fillFrom, fill_index = fill_with_index](const Descriptor &self) -> std::unique_ptr<Raster> {
        std::unique_ptr<Raster> out = Raster::createRaster(self.dataType, self.tileResolution);
        RasterOperations::callUnary<FakeSourceWriter>(out.get(), fillFrom, res_left_to_fill, index, self.nodata, fill_index);
        return out;
    };

    TemporalReference tempInfo(time, time + timeDuration);
    SpatialTemporalReference rasterInfo = qrect;
    rasterInfo.t1 = tempInfo.t1;
    rasterInfo.t2 = tempInfo.t2;

    return rts::make_optional<Descriptor>(std::move(getter), rasterInfo, tile_spat, qrect.tileRes, qrect.order, tileIndex, tileCount, nodata, dataType);
}

bool FakeSource::supportsOrder(Order o) const {
    return o == Order::Spatial || o == Order::Temporal;
}

void FakeSource::increaseCurrentTime(double &currTime) {
    currTime += timeDuration;
}

double FakeSource::getCurrentTimeEnd(double currTime) const {
    return currTime + timeDuration;
}

Origin FakeSource::getOrigin() const {
    return origin;
}
