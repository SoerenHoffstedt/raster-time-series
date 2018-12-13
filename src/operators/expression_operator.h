
#ifndef RASTER_TIME_SERIES_EXPRESSION_OPERATOR_H
#define RASTER_TIME_SERIES_EXPRESSION_OPERATOR_H

#include "operators/generic_operator.h"
#include "util/expression.h"

namespace rts {

    class ExpressionOperator : public GenericOperator {
    public:
        ExpressionOperator(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        OptionalDescriptor nextDescriptor() override;
        bool supportsOrder(Order order) const override;
    private:
        Expression expression;
    };

}

#endif //RASTER_TIME_SERIES_EXPRESSION_OPERATOR_H
