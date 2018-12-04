
#ifndef RASTER_TIME_SERIES_AGGREGATOR_H
#define RASTER_TIME_SERIES_AGGREGATOR_H

#include "generic_operator.h"

namespace rts {

    class Aggregator : public GenericOperator {
    public:
        Aggregator(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        OptionalDescriptor nextDescriptor() override;
        bool supportsOrder(Order order) override;
    private:
        OptionalDescriptor nextDesc;
    };

}


#endif //RASTER_TIME_SERIES_AGGREGATOR_H
