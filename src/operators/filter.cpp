
#include "filter.h"

using namespace rts;

Filter::Filter(Json::Value &params) : GenericOperator(params) {

}

std::unique_ptr<TimeSeries> Filter::createTimeSeries(std::vector<TimeSeries *> &inputs,
                                                     std::shared_ptr<GenericOperator> op_ptr) {
    checkInputCount(inputs, 1);

    std::string type = params["type"].asString();

    double from = params["from"].asDouble();
    double to   = params["to"].asDouble();

    std::unique_ptr<TimeSeries> ts = make_unique<TimeSeries>();

    TimeSeries* input = inputs[0];

    for(Descriptor &in : *input){
        double time_end = in.time_start + in.time_duration;
        //simply copy all descriptors in filtered range, nothing for their source etc has to change.
         if(in.time_start >= from && in.time_start <= to || time_end >= from && time_end <= to) {
            ts->add(in);
        }
    }

    return ts;
}

Raster *Filter::executeOnRaster(Descriptor *descriptor) {
    return nullptr;
}
