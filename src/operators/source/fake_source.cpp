
#include "fake_source.h"
#include <fstream>
#include <iostream>
#include <string>


using namespace rts;


FakeSource::FakeSource(Json::Value &params) : GenericOperator(params) {

}

std::unique_ptr<TimeSeries> FakeSource::createTimeSeries(std::vector<rts::TimeSeries *> &inputs,
                                                         std::shared_ptr<GenericOperator> op_ptr) {
    checkInputCount(inputs, 0);

    UniquePtrTimeSeries ts = make_unique<TimeSeries>();

    Json::Value dataset_json = loadDatasetJson(params["dataset"].asString());

    const int raster_count = dataset_json["raster_count"].asInt();
    double time_start = dataset_json["time_start"].asDouble();
    const double time_duration = dataset_json["time_duration"].asDouble();

    const int res_x = dataset_json["resolution_x"].asInt();
    const int res_y = dataset_json["resolution_y"].asInt();

    Json::Value rect_json = params["rect"];

    //MetaData metaData(rect_json["x1"].asDouble(), rect_json["x2"].asDouble(), rect_json["y1"].asDouble(), rect_json["y2"].asDouble());

    for(int i = 0; i < raster_count; i++) {
        //add rasters descriptors
        std::vector<Descriptor*> input_descriptors;
        Descriptor d(i, res_x, res_y, time_start, time_duration, input_descriptors, op_ptr);
        time_start += time_duration;
        ts->add(d);
    }
    return ts;
}

Raster* FakeSource::executeOnRaster(Descriptor *descriptor) {
    const int res_x = descriptor->res_x;
    const int res_y = descriptor->res_y;
    const int i = descriptor->getId();
    std::cout << "Fake Source: Load Raster " << i << std::endl;
    auto raster = new Raster(res_x, res_y);

    for(int x = 0; x < res_x; x++){
        for(int y = 0; y < res_y; y++) {
            raster->setCell(x, y, i);
        }
    }

    return raster;
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
