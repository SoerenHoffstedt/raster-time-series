
#include "datatypes/raster_operations.h"
#include "operators/expression_operator.h"

using namespace rts;

ExpressionOperator::ExpressionOperator(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
        : GenericOperator(qrect, params, std::move(in)), expression(params["expression"])
{
    checkInputCount(1);
}

OptionalDescriptor ExpressionOperator::nextDescriptor() {

    OptionalDescriptor in = input_operators[0]->nextDescriptor();

    if(!in) {
        return std::nullopt;
    }

    DescriptorInfo descInfo(in);

    /*std::vector<OptionalDescriptor> inputs;
    inputs.reserve(1);
    inputs.push_back(std::move(in));

    auto getter = expression.createGetter(std::move(inputs));*/

    auto getter = [in_desc = std::move(in)](const Descriptor &self) -> std::unique_ptr<Raster> {
        UniqueRaster test = in_desc->getRaster();
        RasterOperations::callUnary<RasterOperations::Squarer>(test.get(), self);
        return test;
    };

    return std::make_optional<Descriptor>(std::move(getter), descInfo);
}

bool ExpressionOperator::supportsOrder(Order order) const {
    return order == Order::Temporal || order == Order::Spatial;
}
