
#include <unordered_map>
#include <iostream>
#include "operators/generic_operator.h"
#include "operators/print.h"
#include "operators/source/fake_source.h"
#include "generic_operator.h"


using namespace rts;

GenericOperator::GenericOperator(QueryRectangle qrect, Json::Value &params, const std::vector<GenericOperator*> &in)
    : qrect(qrect), params(params), input_operators(in)
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

UniqueDescriptor OperatorUtil::skipCurrentDimension(GenericOperator &op, UniqueDescriptor currentDesc) {
    if(currentDesc->totalInfo.order == Order::SpatialTemporal)
        return skipCurrentSpatial(op, std::move(currentDesc));
    else if(currentDesc->totalInfo.order == Order::TemporalSpatial)
        return skipCurrentTemporal(op, std::move(currentDesc));
}

UniqueDescriptor OperatorUtil::skipCurrentTemporal(GenericOperator &op, UniqueDescriptor currentDesc) {
    UniqueDescriptor returnDesc = nullptr;
    if(currentDesc == nullptr)
        return returnDesc;

    double time1_to_skip = currentDesc->tileInfo.t1;
    double time2_to_skip = currentDesc->tileInfo.t2;

    while(true){
        returnDesc = op.next();
        if(returnDesc == nullptr || (returnDesc->totalInfo.t1 != time1_to_skip || returnDesc->totalInfo.t2 != time2_to_skip))
            break;
    }

    return returnDesc;
}

UniqueDescriptor OperatorUtil::skipCurrentSpatial(GenericOperator &op, UniqueDescriptor currentDesc) {
    UniqueDescriptor returnDesc = nullptr;

    //TODO: make this more robust by checking the spatial reference
    double last_t1 = currentDesc->tileInfo.t1;

    while(true){
        returnDesc = op.next();
        if(returnDesc == nullptr || currentDesc->tileInfo.t1 <= last_t1)
            break;
    }

    return returnDesc;
}
