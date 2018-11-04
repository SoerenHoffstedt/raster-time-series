
#include <unordered_map>
#include <iostream>
#include "operators/generic_operator.h"
#include "operators/print.h"
#include "operators/source/fake_source.h"
#include "generic_operator.h"


using namespace rts;

GenericOperator::GenericOperator(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
    : qrect(qrect), params(params), input_operators(std::move(in))
{

}

GenericOperator::GenericOperator(QueryRectangle qrect, Json::Value &params) : qrect(qrect), params(params) {

}

void GenericOperator::checkInputCount(int expected) {
    if(input_operators.size() != expected)
        throw std::runtime_error("Operator has unexpected amount of input operators.");
}

TimeSeriesIterator GenericOperator::begin() {
    return TimeSeriesIterator(dynamic_cast<GenericOperator *>(this));
}

TimeSeriesIterator GenericOperator::end() {
    return TimeSeriesIterator::createEndIterator();
}

OptionalDescriptor OperatorUtil::skipCurrentDimension(GenericOperator &op, OptionalDescriptor &currentDesc) {
    if(currentDesc->order == Order::SpatialTemporal)
        return skipCurrentSpatial(op, currentDesc);
    else if(currentDesc->order == Order::TemporalSpatial)
        return skipCurrentTemporal(op, currentDesc);
}

OptionalDescriptor OperatorUtil::skipCurrentTemporal(GenericOperator &op, OptionalDescriptor &currentDesc) {
    OptionalDescriptor returnDesc = std::nullopt;
    if(!currentDesc)
        return returnDesc;

    double time1_to_skip = currentDesc->rasterInfo.t1;
    double time2_to_skip = currentDesc->rasterInfo.t2;

    while(true){
        returnDesc = op.next();
        if(!returnDesc || (returnDesc->rasterInfo.t1 != time1_to_skip || returnDesc->rasterInfo.t2 != time2_to_skip))
            break;
    }

    return returnDesc;
}

OptionalDescriptor OperatorUtil::skipCurrentSpatial(GenericOperator &op, OptionalDescriptor &currentDesc) {
    OptionalDescriptor returnDesc = std::nullopt;

    //TODO: make this more robust by checking the spatial reference: better tileIndex.
    double last_t1 = currentDesc->rasterInfo.t1;

    while(true){
        returnDesc = op.next();
        if(!returnDesc || currentDesc->rasterInfo.t1 <= last_t1)
            break;
    }

    return returnDesc;
}
