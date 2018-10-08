
#ifndef RASTER_TIME_SERIES_CONSUMING_OPERATOR_H
#define RASTER_TIME_SERIES_CONSUMING_OPERATOR_H

#include "operators/generic_operator.h"

namespace rts {

    class ConsumingOperator : public GenericOperator {
    public:
        ConsumingOperator(Json::Value &params, const std::vector<GenericOperator*> &in) : GenericOperator(params, in) { }
        ConsumingOperator(Json::Value &params) : GenericOperator(params) { }
        virtual void consume() = 0;
    };

}

#endif //RASTER_TIME_SERIES_CONSUMING_OPERATOR_H
