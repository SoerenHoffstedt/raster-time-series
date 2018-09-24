
#ifndef RASTER_TIME_SERIES_EXPRESSION_H
#define RASTER_TIME_SERIES_EXPRESSION_H

#include "operators/generic_operator.h"

namespace rts {

    class Expression : public GenericOperator {
    public:
        Expression(Json::Value &params, const std::vector<GenericOperator *> &in);
        UniqueDescriptor next() override;

    };

}

#endif //RASTER_TIME_SERIES_EXPRESSION_H
