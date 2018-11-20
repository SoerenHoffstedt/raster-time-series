
#include <fstream>
#include <iostream>
#include <string>
#include <functional>
#include <filesystem>
#include "datatypes/descriptor.h"
#include "util/raster_calculations.h"
#include "operators/source/fake_source.h"

using namespace rts;

FakeSource::FakeSource(QueryRectangle qrect,Json::Value &params) : GenericOperator(qrect, params), rasterIndex(0), tileIndex(0) {
    dataset_json = loadDatasetJson(params["dataset"].asString());
    raster_count = dataset_json["raster_count"].asInt();
    time_start = dataset_json["time_start"].asDouble();
    time_curr = time_start;
    time_duration = dataset_json["time_duration"].asDouble();
    nodata = dataset_json["nodata"].asDouble();
    dataType = GDT_Int32;
    state_x = 0;
    state_y = 0;
    tile_res.res_x = params["tile_size_x"].asUInt();
    tile_res.res_y = params["tile_size_y"].asUInt();
    if(dataset_json.isMember("spatial_reference")){
        SpatialReference sref(dataset_json["spatial_reference"]);
        this->qrect.x1 = sref.x1;
        this->qrect.x2 = sref.x2;
        this->qrect.y1 = sref.y1;
        this->qrect.y2 = sref.y2;
    }
    rasterWorldPixelStart = RasterCalculations::coordinateToWorldPixel(qrect, qrect.x1, qrect.y1);

    //calc number of tiles
    Resolution rasterStep = rasterWorldPixelStart;
    rasterStep.res_x -= rasterWorldPixelStart.res_x % tile_res.res_x;
    rasterStep.res_y -= rasterWorldPixelStart.res_y % tile_res.res_y;
    Resolution rasterWorldPixelEnd = RasterCalculations::coordinateToWorldPixel(qrect, qrect.x2, qrect.y2);

    for(tileCount = 0; rasterStep.res_x >= rasterWorldPixelEnd.res_x &&  rasterStep.res_y >= rasterWorldPixelEnd.res_y; tileCount++){
        if(rasterStep.res_x >= rasterWorldPixelEnd.res_x){
            rasterStep.res_y += tile_res.res_y;
        } else {
            rasterStep.res_x += tile_res.res_x;
        }
    }
}

Json::Value FakeSource::loadDatasetJson(std::string name) {
    std::filesystem::path p("../../test/data/");
    p /= std::filesystem::path(name + ".json");
    std::ifstream file_in(p.string());
    Json::Value dataset_json;
    file_in >> dataset_json;
    return dataset_json;
}

OptionalDescriptor FakeSource::nextDescriptor() {

    if(rasterIndex >= raster_count)
        return std::nullopt;

    while(time_curr < qrect.t1){
        time_curr += time_duration;
        rasterIndex += 1;
        if(rasterIndex >= raster_count)
            return std::nullopt;
    }

    if(time_curr > qrect.t2){
        return std::nullopt;
    }
    if(state_y >= static_cast<int>(qrect.res_y)){ //when state_y is not reset, the end is reached.
        return std::nullopt;
    }

    Resolution fill_from(0, 0);

    //fill_from: for fixed alignment of tiles, start of a tile is not always 0, based on what pixel in world space the tile starts.
    if(state_x == 0) {
        fill_from.res_x = rasterWorldPixelStart.res_x % tile_res.res_x;
        state_x -= fill_from.res_x; //state_x will have tile_res.res_x added later on, so align it to the tiles here.
    } else if(state_x < 0){
        fill_from.res_x = rasterWorldPixelStart.res_x % tile_res.res_x;
    }
    if(state_y == 0) {
        fill_from.res_y = rasterWorldPixelStart.res_y % tile_res.res_y;
        state_y -= fill_from.res_y;
    } else if(state_y < 0){
        fill_from.res_y = rasterWorldPixelStart.res_y % tile_res.res_y;
    }
    Resolution res_left_to_fill(qrect.res_x - state_x, qrect.res_y - state_y);

    Resolution tile_start_world_res(rasterWorldPixelStart.res_x + state_x, rasterWorldPixelStart.res_y + state_y);
    SpatialReference tile_spat = RasterCalculations::calcSpatialInfoFromTilePixel(qrect, tile_start_world_res, tile_start_world_res + tile_res);

    auto getter = [index = rasterIndex, tile_res = tile_res, res_left_to_fill = res_left_to_fill, fill_from = fill_from, nodata = nodata](const Descriptor &self) -> std::unique_ptr<Raster> {
        std::unique_ptr<Raster> out = std::make_unique<Raster>(tile_res);

        for (int x = 0; x < tile_res.res_x; ++x) {
            for (int y = 0; y < tile_res.res_y; ++y) {

                if(x >= fill_from.res_x && y >= fill_from.res_y && x < res_left_to_fill.res_x && y < res_left_to_fill.res_y)
                    out->setCell(x, y, index);
                else
                    out->setCell(x,y, nodata);
            }
        }
        return out;
    };

    TemporalReference tempInfo(time_curr, time_curr + time_duration);
    int tileIndexNow = tileIndex;

    if(qrect.order == Order::Temporal){
        if(increaseSpatial()){
            increaseTemporal();
        }
    } else if(qrect.order == Order::Spatial){
        if(increaseTemporal()){
            increaseSpatial();
        }
    }

    SpatialTemporalReference rasterInfo = qrect;
    rasterInfo.t1 = tempInfo.t1;
    rasterInfo.t2 = tempInfo.t2;

    // qrect_total, qrect_tile
    return std::make_optional<Descriptor>(std::move(getter), rasterInfo, tile_spat, tile_res, qrect.order, tileIndexNow, tileCount, nodata, dataType);
}

bool FakeSource::supportsOrder(Order o) {
    return o == Order::Spatial || o == Order::Temporal;
}

bool FakeSource::increaseTemporal() {
    rasterIndex += 1;
    time_curr += time_duration;
    if(time_curr >= qrect.t2){
        if(qrect.order == Order::Spatial){
            //reset only if temporal is the outer dimension, else keep time_curr above t2 as end condition at top of nextDescriptor() method
            rasterIndex = 0;
            time_curr = time_start;
            return true;
        }
    }
    return false;
}

bool FakeSource::increaseSpatial() {
    tileIndex += 1;
    state_x += tile_res.res_x;
    if(state_x >= static_cast<int>(qrect.res_x)){
        state_x = 0;
        state_y += tile_res.res_y;
        if(state_y >= static_cast<int>(qrect.res_y)){
            if(qrect.order == Order::Temporal){
                //reset only if spatial is the outer dimension, else keep state_y as end condition at start of nextDescriptor() method.
                tileIndex = 0;
                state_y = 0;
                return true;
            }
        }
    }
    return false;
}
