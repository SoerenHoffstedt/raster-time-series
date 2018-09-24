
#include <fstream>
#include <iostream>
#include <string>
#include <functional>
#include <datatypes/descriptor.h>
#include "operators/source/fake_source.h"


using namespace rts;

FakeSource::FakeSource(Json::Value &params) : GenericOperator(params), index(0) {
    dataset_json = loadDatasetJson(params["dataset"].asString());
    raster_count = dataset_json["raster_count"].asInt();
    time_start = dataset_json["time_start"].asDouble();
    time_duration = dataset_json["time_duration"].asDouble();
    res_x = dataset_json["resolution_x"].asInt();
    res_y = dataset_json["resolution_y"].asInt();
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

    auto getter = [index = index](Descriptor *self) -> std::unique_ptr<Raster> {
        std::unique_ptr<Raster> out = make_unique<Raster>(self->st_ref.res_x, self->st_ref.res_y);
        for (int x = 0; x < self->st_ref.res_x; ++x) {
            for (int y = 0; y < self->st_ref.res_y; ++y) {
                out->setCell(x, y, index);
            }
        }
        return out;
    };

    const double time_start_curr = time_start;
    index += 1;
    time_start += time_duration;

    return rts::createUniqueDescriptor(std::move(getter),
            TemporalReference(time_start_curr, time_start_curr + time_duration),
            SpatialReference(-180.0, 180.0, -90.0, 90.0),
            Resolution(res_x, res_y));
}
