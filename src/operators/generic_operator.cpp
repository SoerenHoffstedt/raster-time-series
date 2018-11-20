
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
    if(currentDesc->order == Order::Spatial)
        return skipCurrentSpatial(op, currentDesc);
    if(currentDesc->order == Order::Temporal)
        return skipCurrentTemporal(op, currentDesc);
}

OptionalDescriptor OperatorUtil::skipCurrentTemporal(GenericOperator &op, OptionalDescriptor &currentDesc) {
    OptionalDescriptor returnDesc = std::nullopt;
    if(!currentDesc)
        return returnDesc;

    double time1_to_skip = currentDesc->rasterInfo.t1;
    double time2_to_skip = currentDesc->rasterInfo.t2;

    while(true){
        returnDesc = op.nextDescriptor();
        if(!returnDesc || (returnDesc->rasterInfo.t1 != time1_to_skip || returnDesc->rasterInfo.t2 != time2_to_skip))
            break;
    }

    return returnDesc;
}

OptionalDescriptor OperatorUtil::skipCurrentSpatial(GenericOperator &op, OptionalDescriptor &currentDesc) {
    OptionalDescriptor returnDesc = op.nextDescriptor();
    const int tileIndex = currentDesc->tileIndex;
    while(returnDesc.has_value() && currentDesc->tileIndex == tileIndex){
        returnDesc = op.nextDescriptor();
    }
    return returnDesc;
}
