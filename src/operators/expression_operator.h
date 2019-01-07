
#ifndef RASTER_TIME_SERIES_EXPRESSION_OPERATOR_H
#define RASTER_TIME_SERIES_EXPRESSION_OPERATOR_H

#include "operators/generic_operator.h"
#include "util/expression.h"

namespace rts {

    /**
     * Operator for calculating expressions with input rasters.
     * How many input rasters are supported is defined by the Expression class (see util/expression.h/cpp).
     * This operator simply calculates with the tiles that come as input. If calculations should be done
     * based on their temporal overlap, use the TemporalOverlap operator.
     */
    class ExpressionOperator : public GenericOperator {
    public:
        ExpressionOperator(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        OptionalDescriptor nextDescriptor() override;
        void initialize() override;
        bool supportsOrder(Order order) const override;
    private:
        Expression expression;
    };

}

#endif //RASTER_TIME_SERIES_EXPRESSION_OPERATOR_H
