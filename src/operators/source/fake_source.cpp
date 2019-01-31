
#include <fstream>
#include <iostream>
#include <string>
#include <functional>
#include <filesystem>
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

FakeSource::FakeSource(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, UniqueOperatorVector &&in)
        : SourceOperator(operator_tree, qrect, params, std::move(in)) {

}

void FakeSource::initialize() {
    auto dataset_json = loadDatasetJson(params["dataset"].asString());
    rasterCount = dataset_json["raster_count"].asInt();
    datasetStartTime = dataset_json["time_start"].asDouble();
    datasetEndTime = std::numeric_limits<double>::max();
    timeDuration = dataset_json["time_duration"].asDouble();
    setCurrTimeToFirstRaster();

    nodata = dataset_json["nodata"].asDouble();
    dataType = Parsing::parseDataType(dataset_json["data_type"].asString());

    if(dataset_json.isMember("spatial_reference")){
        SpatialReference sref(dataset_json["spatial_reference"]);
        this->qrect.x1 = sref.x1;
        this->qrect.x2 = sref.x2;
        this->qrect.y1 = sref.y1;
        this->qrect.y2 = sref.y2;
    }

    //calc number of tiles
    rasterWorldPixelStart = RasterCalculations::coordinateToPixel(qrect, qrect.x1, qrect.y1);
    origin.x = qrect.x1;
    origin.y = qrect.y1;

    Resolution rasterStep = rasterWorldPixelStart;
    rasterStep.resX -= rasterWorldPixelStart.resX % qrect.tileRes.resX;
    rasterStep.resY -= rasterWorldPixelStart.resY % qrect.tileRes.resY;
    Resolution rasterWorldPixelEnd = RasterCalculations::coordinateToPixel(qrect, qrect.x2, qrect.y2);
    Resolution size(rasterWorldPixelEnd.resX - rasterStep.resX, rasterWorldPixelEnd.resY - rasterStep.resY);
    tileCount.resX = size.resX / qrect.tileRes.resX;
    tileCount.resY = size.resY / qrect.tileRes.resY;
    if(size.resX % qrect.tileRes.resX > 0)
        tileCount.resX += 1;
    if(size.resY % qrect.tileRes.resY > 0)
        tileCount.resY += 1;

    extent = qrect.projection.getExtent();
    fill_with_index = params.get("fill_with_index", false).asBool();
}

Json::Value FakeSource::loadDatasetJson(std::string name) {
    std::filesystem::path p("../../test/data/fake_source");
    p /= std::filesystem::path(name + ".json");
    std::ifstream file_in(p.string());
    Json::Value dataset_json;
    file_in >> dataset_json;
    return dataset_json;
}

OptionalDescriptor FakeSource::createDescriptor(double time, int pixelStartX, int pixelStartY, int tileIndex) {

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
    SpatialReference tile_spat = RasterCalculations::pixelToSpatialRectangle(qrect, tile_start_world_res,
                                                                             tile_start_world_res + qrect.tileRes);

    auto getter = [index = currRasterIndex, res_left_to_fill = res_left_to_fill, fillFrom = fillFrom, fill_index = fill_with_index](const Descriptor &self) -> std::unique_ptr<Raster> {
        std::unique_ptr<Raster> out = Raster::createRaster(self.dataType, self.tileResolution);
        RasterOperations::callUnary<FakeSourceWriter>(out.get(), fillFrom, res_left_to_fill, index, self.nodata, fill_index);
        return out;
    };

    TemporalReference tempInfo(time, time + timeDuration);
    SpatialTemporalReference rasterInfo = qrect;
    rasterInfo.t1 = tempInfo.t1;
    rasterInfo.t2 = tempInfo.t2;

    return std::make_optional<Descriptor>(std::move(getter), rasterInfo, tile_spat, qrect.tileRes, qrect.order, tileIndex, tileCount, nodata, dataType);
}

bool FakeSource::supportsOrder(Order o) const {
    return o == Order::Spatial || o == Order::Temporal;
}

void FakeSource::increaseCurrentTime() {
    currTime += timeDuration;
}

double FakeSource::getCurrentTimeEnd() const {
    return currTime + timeDuration;
}
