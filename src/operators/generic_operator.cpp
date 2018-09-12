
#include <unordered_map>
#include <iostream>
#include "operators/generic_operator.h"
#include "generic_operator.h"
#include "operators/source/fake_source.h"
#include "operators/expression.h"
#include "operators/filter.h"

rts::GenericOperator::GenericOperator(Json::Value &params) : params(params) {

}

void rts::GenericOperator::checkInputCount(std::vector<TimeSeries*> &inputs, int expected) {
    if(inputs.size() != expected)
        throw std::runtime_error("Operator got wrong number of inputs.");
}

std::shared_ptr<rts::GenericOperator> rts::GenericOperator::getSharedOperator(const std::string &op_name, Json::Value &params) {

    //TODO: replace with automatic registration.
    if(op_name == "fake_source"){
        return std::dynamic_pointer_cast<GenericOperator>(std::make_shared<rts::FakeSource>(params));
    } else if(op_name == "expression"){
        return std::dynamic_pointer_cast<GenericOperator>(std::make_shared<rts::ExpressionOperator>(params));
    } else if(op_name == "filter"){
        return std::dynamic_pointer_cast<GenericOperator>(std::make_shared<rts::Filter>(params));
    }

}
