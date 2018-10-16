
#ifndef RASTER_TIME_SERIES_CONSUMING_OPERATOR_H
#define RASTER_TIME_SERIES_CONSUMING_OPERATOR_H

#include "operators/generic_operator.h"

namespace rts {

    class ConsumingOperator : public GenericOperator {
    public:
        ConsumingOperator(QueryRectangle qrect, Json::Value &params, const std::vector<GenericOperator*> &in) : GenericOperator(qrect, params, in) { }
        ConsumingOperator(QueryRectangle qrect, Json::Value &params) : GenericOperator(qrect, params) { }
        virtual void consume() = 0;
    };

}

#endif //RASTER_TIME_SERIES_CONSUMING_OPERATOR_H
