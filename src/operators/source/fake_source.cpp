
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
    if(dataset_json.isMember("spatial_reference")){
        SpatialReference sref(dataset_json["spatial_reference"]);
        this->qrect.x1 = sref.x1;
        this->qrect.x2 = sref.x2;
        this->qrect.y1 = sref.y1;
        this->qrect.y2 = sref.y2;
    }
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

OptionalDescriptor FakeSource::next() {

    if(index >= raster_count)
        return std::nullopt;

    while(time_curr < qrect.t1){
        time_curr += time_duration;
        index += 1;
        if(index >= raster_count)
            return std::nullopt;
    }

    if(time_curr > qrect.t2){
        return std::nullopt;
    }
    if(state_y >= qrect.res_y){ //when state_y is not reset, the end is reached.
        return std::nullopt;
    }

    Resolution res_left_to_fill(qrect.res_x - state_x, qrect.res_y - state_y);

    auto getter = [index = index, tile_res = tile_res, res_left_to_fill = res_left_to_fill, nodata = nodata](const Descriptor &self) -> std::unique_ptr<Raster> {
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
    SpatialReference spat_tile = getCoordsForTile(qrect, qrect, tile_res, Resolution(state_x, state_y));

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
    return std::make_optional<Descriptor>(
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

//TODO: Move this somewhere else, so it is reusable at different points. Maybe in SpatialReference class?
SpatialReference FakeSource::getCoordsForTile(const SpatialReference &totalCoords, const Resolution &totalRes, const Resolution &tileRes, const Resolution &startOfTile)
{
    if(totalRes.equalsResolution(tileRes))
        return totalCoords;
    SpatialReference res;

    int x1 = startOfTile.res_x;
    int x2 = x1 + tileRes.res_x;
    int y1 = startOfTile.res_y;
    int y2 = y1 + tileRes.res_y;

    double perc_x1 = (float)x1 / totalRes.res_x;
    double perc_x2 = (float)x2 / totalRes.res_x;
    double perc_y1 = (float)y1 / totalRes.res_y;
    double perc_y2 = (float)y2 / totalRes.res_y;

    double coord_length_x = totalCoords.x2 - totalCoords.x1;
    double coord_length_y = totalCoords.y2 - totalCoords.y1;

    res.x1 = totalCoords.x1 + coord_length_x * perc_x1;
    res.x2 = totalCoords.x1 + coord_length_x * perc_x2;
    res.y1 = totalCoords.y1 + coord_length_y * perc_y1;
    res.y2 = totalCoords.y1 + coord_length_y * perc_y2;

    return res;
}
