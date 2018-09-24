
#include <unordered_map>
#include <iostream>
#include "operators/generic_operator.h"
#include "operators/print.h"
#include "operators/source/fake_source.h"

using namespace rts;

GenericOperator::GenericOperator(Json::Value &params, const std::vector<GenericOperator*> &in)
    : input_operators(in), params(params)
{

}

void GenericOperator::in(GenericOperator *op) {
    input_operators.push_back(op);
}

GenericOperator::GenericOperator(Json::Value &params) : params(params) {

}

void GenericOperator::checkInputCount(int expected) {
    if(input_operators.size() != expected)
        throw std::runtime_error("Operator has unexpected amount of input operators.");
}


