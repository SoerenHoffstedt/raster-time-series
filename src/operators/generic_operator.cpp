
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


