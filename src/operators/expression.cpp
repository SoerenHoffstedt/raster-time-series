
#include "datatypes/raster_operations.h"
#include "expression.h"

using namespace rts;

Expression::Expression(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in) : GenericOperator(qrect, params, std::move(in))
{
    checkInputCount(1);
}

OptionalDescriptor Expression::nextDescriptor() {
    OptionalDescriptor in = input_operators[0]->nextDescriptor();

    if(!in) {
        return std::nullopt;
    }

    DescriptorInfo descInfo(in);

    auto getter = [in_desc = std::move(in)](const Descriptor &self) -> std::unique_ptr<Raster> {
        UniqueRaster test = in_desc->getRaster();
        RasterOperations::callUnary<RasterOperations::Squarer>(test.get(), self);
        return test;
    };

    return std::make_optional<Descriptor>(std::move(getter), descInfo);
}

bool Expression::supportsOrder(Order order) {
    return order == Order::Temporal || order == Order::Spatial;
}
