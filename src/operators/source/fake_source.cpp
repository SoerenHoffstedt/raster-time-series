
#include <fstream>
#include <iostream>
#include <string>
#include <functional>
#include <datatypes/descriptor.h>
#include "operators/source/fake_source.h"
#include "fake_source.h"


using namespace rts;

FakeSource::FakeSource(QueryRectangle qrect,Json::Value &params) : GenericOperator(qrect, params), index(0) {
    dataset_json = loadDatasetJson(params["dataset"].asString());
    raster_count = dataset_json["raster_count"].asInt();
    time_start = dataset_json["time_start"].asDouble();
    time_curr = time_start;
    time_duration = dataset_json["time_duration"].asDouble();
    nodata = dataset_json["nodata"].asInt();
    state_x = 0;
    state_y = 0;
    tile_res.res_x = params["tile_size_x"].asInt();
    tile_res.res_y = params["tile_size_y"].asInt();
}

Json::Value FakeSource::loadDatasetJson(std::string name) {
    std::string path = "../../test/data/";
    path += name;
    path += ".json";
    std::ifstream file_in(path);
    Json::Value dataset_json;
    file_in >> dataset_json;
    return dataset_json;
}

UniqueDescriptor FakeSource::next() {

    if(index >= raster_count)
        return nullptr;

    while(time_curr < qrect.t1){
        time_curr += time_duration;
        index += 1;
        if(index >= raster_count)
            return nullptr;
    }

    if(time_curr > qrect.t2){
        return nullptr;
    }
    if(state_y >= qrect.res_y){ //when state_y is not reset, the end is reached.
        return nullptr;
    }

    Resolution res_left_to_fill(qrect.res_x - state_x, qrect.res_y - state_y);

    auto getter = [index = index, tile_res = tile_res, res_left_to_fill = res_left_to_fill, nodata = nodata](Descriptor *self) -> std::unique_ptr<Raster> {
        std::unique_ptr<Raster> out = std::make_unique<Raster>(tile_res);

        for (int x = 0; x < tile_res.res_x; ++x) {
            for (int y = 0; y < tile_res.res_y; ++y) {

                if(x < res_left_to_fill.res_x && y < res_left_to_fill.res_y)
                    out->setCell(x, y, index);
                else
                    out->setCell(x,y, nodata);
            }
        }
        return out;
    };

    TemporalReference temp_tile(time_curr, time_curr + time_duration);
    SpatialReference spat_tile(state_x, state_x + tile_res.res_x, state_y, state_y + tile_res.res_y);

    TemporalReference temp_total;
    SpatialReference spat_total;

    if(qrect.order == Order::TemporalSpatial){

        temp_total = temp_tile;
        spat_total = qrect;//is different

        if(increaseSpatial()){
            increaseTemporal();
        }
    } else if(qrect.order == Order::SpatialTemporal){
        temp_total = qrect;
        spat_total = spat_tile;

        if(increaseTemporal()){
            increaseSpatial();
        }
    }

    // qrect_total, qrect_tile
    return rts::createUniqueDescriptor(
            std::move(getter),
            QueryRectangle(temp_total, spat_total, Resolution(qrect.res_x, qrect.res_y), qrect.order),
            QueryRectangle(temp_tile, spat_tile, tile_res, qrect.order));
}

bool FakeSource::supportsOrder(Order o) {
    return o == Order::SpatialTemporal || o == Order::TemporalSpatial;
}

bool FakeSource::increaseTemporal() {
    index += 1;
    time_curr += time_duration;
    if(time_curr >= qrect.t2){
        if(qrect.order == Order::SpatialTemporal){
            //reset only if temporal is the outer dimension, else keep time_curr above t2 as end condition at top of next() method
            index = 0;
            time_curr = time_start;
            return true;
        }
    }
    return false;
}

bool FakeSource::increaseSpatial() {
    state_x += tile_res.res_x;
    if(state_x >= qrect.res_x){
        state_x = 0;
        state_y += tile_res.res_y;
        if(state_y >= qrect.res_y){
            if(qrect.order == Order::TemporalSpatial){
                //reset only if spatial is the outer dimension, else keep state_y as end condition at start of next() method.
                state_y = 0;
                return true;
            }
        }
    }
    return false;
}
