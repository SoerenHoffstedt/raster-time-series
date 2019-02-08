
#include <unordered_map>
#include <iostream>
#include "operators/generic_operator.h"
#include "operators/consuming/print.h"
#include "operators/source/fake_source.h"
#include "generic_operator.h"


using namespace rts;

GenericOperator::GenericOperator(const OperatorTree *operator_tree, const  QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
    : operator_tree(operator_tree), qrect(qrect), params(params), input_operators(std::move(in))
{

}

void GenericOperator::checkInputCount(int expected) const {
    if(input_operators.size() != expected)
        throw std::runtime_error("Operator has unexpected amount of input operators.");
}

void GenericOperator::checkInputCount(int expectedMin, int expectedMax) const {
    if(input_operators.size() < expectedMin || input_operators.size() > expectedMax)
        throw std::runtime_error("Operator has unexpected amount of input operators.");
}
TimeSeriesIterator GenericOperator::begin() {
    return TimeSeriesIterator(dynamic_cast<GenericOperator *>(this));
}

TimeSeriesIterator GenericOperator::end() {
    return TimeSeriesIterator::createEndIterator();
}

void GenericOperator::initializeRecursively() {
    callInitializeRecursively(this);
}

void GenericOperator::callInitializeRecursively(GenericOperator *op){
    op->initialize();
    for(auto &child : op->input_operators){
        callInitializeRecursively(child.get());
    }
}

void GenericOperator::skipCurrentRaster(const uint32_t skipCount) {
    for(auto &input_op : input_operators){
        input_op->skipCurrentRaster(skipCount);
    }
}

void GenericOperator::skipCurrentTile(const uint32_t skipCount) {
    for(auto &input_op : input_operators){
        input_op->skipCurrentTile(skipCount);
    }
}

std::unique_ptr<GenericOperator> GenericOperator::reInstantiate(const QueryRectangle &qrect) const {
    auto clone = operator_tree->instantiate();
    setQrectRecursively(clone.get(), qrect);
    clone->initializeRecursively();
    return clone;
}

void GenericOperator::setQrectRecursively(GenericOperator *op, const QueryRectangle &qrect) const {
    op->qrect = qrect;
    for(auto &in : op->input_operators){
        setQrectRecursively(in.get(), qrect);
    }
}
