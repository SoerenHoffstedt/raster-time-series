
#ifndef RASTER_TIME_SERIES_CONSUMING_OPERATOR_H
#define RASTER_TIME_SERIES_CONSUMING_OPERATOR_H

#include "operators/generic_operator.h"

namespace rts {

    class GenericOperator;

    class ConsumingOperator : public GenericOperator {
    public:
        ConsumingOperator(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
                : GenericOperator(operator_tree, qrect, params, std::move(in)) { }
        virtual void consume() = 0;
        OptionalDescriptor nextDescriptor() override;
    };

}

#endif //RASTER_TIME_SERIES_CONSUMING_OPERATOR_H
