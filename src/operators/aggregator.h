
#ifndef RASTER_TIME_SERIES_AGGREGATOR_H
#define RASTER_TIME_SERIES_AGGREGATOR_H

#include "generic_operator.h"

namespace rts {

    class Aggregator : public GenericOperator {
    public:
        Aggregator(QueryRectangle qrect, Json::Value &params, const std::vector<GenericOperator*> &in);
        UniqueDescriptor next() override;
        bool supportsOrder(Order order) override;
    };

}


#endif //RASTER_TIME_SERIES_AGGREGATOR_H
