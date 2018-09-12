
#include <iostream>
#include <fstream>
#include <json/json.h>
#include "datatypes/timeseries.h"
#include "operators/source/fake_source.h"
#include "operators/filter.h"
#include "operators/expression.h"


int main() {

    using namespace rts;
    
    std::ifstream file_in("../../test/query/test_query.json");

    Json::Value json_query;
    file_in >> json_query;


    std::vector<TimeSeries*> inputs;
    std::shared_ptr<GenericOperator> source_op = GenericOperator::getSharedOperator("fake_source", json_query["fake_source_params"]);
    UniquePtrTimeSeries timeSeries1 = source_op->createTimeSeries(inputs, source_op);

    std::shared_ptr<GenericOperator> source_op_2 = GenericOperator::getSharedOperator("fake_source", json_query["fake_source_params"]);
    UniquePtrTimeSeries timeSeries2 = source_op_2->createTimeSeries(inputs, source_op_2);

    inputs.push_back(timeSeries1.get());
    inputs.push_back(timeSeries2.get());


    std::shared_ptr<GenericOperator> expression_op = GenericOperator::getSharedOperator("expression", json_query["expression_params"]);
    UniquePtrTimeSeries timeSeries3 = expression_op->createTimeSeries(inputs, expression_op);

    UniquePtrTimeSeries timeSeries4 = nullptr;
    if(false){
        inputs.clear();
        inputs.emplace_back(timeSeries3.get());
        std::shared_ptr<GenericOperator> filter_op = std::dynamic_pointer_cast<GenericOperator>(std::make_shared<Filter>(json_query["filter_params"]));
        timeSeries4 = filter_op->createTimeSeries(inputs, filter_op);
    } else {
        timeSeries4 = timeSeries3->sample(10);
    }

    for(auto &raster_desc : *timeSeries4){
        int res_x = raster_desc.res_x;
        int res_y = raster_desc.res_y;
        std::cout << "Res: " << res_x << ", " << res_y << std::endl;
        std::cout << "Start: " << raster_desc.time_start << std::endl;
        std::cout << "Duration: " << raster_desc.time_duration << std::endl;
        std::cout << "Is raster already loaded: " << raster_desc.isRasterLoaded() << std::endl;
        auto raster_ptr = raster_desc.getRasterPtr();
        raster_ptr->print();
    }

    return 0;
}
