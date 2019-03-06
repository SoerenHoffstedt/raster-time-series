
#include "datatypes/raster_operations.h"
#include "operators/expression_operator.h"

using namespace rts;

ExpressionOperator::ExpressionOperator(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
        : GenericOperator(operator_tree, qrect, params, std::move(in)), expression(params["expression"])
{
    checkInputCount(1, 2);
}

void ExpressionOperator::initialize() {

}

OptionalDescriptor ExpressionOperator::nextDescriptor() {

    std::vector<OptionalDescriptor> inputs;
    inputs.reserve(input_operators.size());
    for(int i = 0; i < input_operators.size(); ++i){
        inputs.emplace_back(input_operators[i]->nextDescriptor());
        //TODO: Can something be calculated when one of the inputs is nullopt?
        if(!inputs[i])
            return boost::none;
    }

    //TODO: what is spatial info, what is temporal info of result?
    //TODO: check if one operator is only nodata? have it behave like a 0 for add and mul?
    DescriptorInfo descInfo(inputs[0]);

    auto getter = expression.createGetter(std::move(inputs));

    return rts::make_optional<Descriptor>(std::move(getter), descInfo);
}

OptionalDescriptor ExpressionOperator::getDescriptor(int tileIndex) {

    std::vector<OptionalDescriptor> inputs;
    inputs.reserve(input_operators.size());
    for(int i = 0; i < input_operators.size(); ++i){
        inputs.emplace_back(input_operators[i]->getDescriptor(tileIndex));
        //TODO: Can something be calculated when one of the inputs is nullopt?
        if(!inputs[i])
            return boost::none;
    }

    //TODO: what is spatial info, what is temporal info of result?
    //TODO: check if one operator is only nodata? have it behave like a 0 for add and mul?
    DescriptorInfo descInfo(inputs[0]);

    auto getter = expression.createGetter(std::move(inputs));

    return rts::make_optional<Descriptor>(std::move(getter), descInfo);
}

bool ExpressionOperator::supportsOrder(Order order) const {
    return order == Order::Temporal || order == Order::Spatial;
}
