
#ifndef RASTER_TIME_SERIES_EXPRESSION_H
#define RASTER_TIME_SERIES_EXPRESSION_H

#include "operators/generic_operator.h"

namespace rts {

    class Expression : public GenericOperator {
    public:
        Expression(QueryRectangle qrect, Json::Value &params, const std::vector<GenericOperator *> &in);
        UniqueDescriptor next() override;
        bool supportsOrder(Order order) override;
    };

}

#endif //RASTER_TIME_SERIES_EXPRESSION_H
