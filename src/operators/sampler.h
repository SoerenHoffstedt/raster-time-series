
#ifndef RASTER_TIME_SERIES_SAMPLER_H
#define RASTER_TIME_SERIES_SAMPLER_H

#include "operators/generic_operator.h"

namespace rts {


    class Sampler : public GenericOperator {
    public:
        Sampler(QueryRectangle qrect, Json::Value &params, const std::vector<GenericOperator*> &in);
        OptionalDescriptor next() override;
        bool supportsOrder(Order order) override;
    private:
        uint32_t toSkip;
        double lastSendT1;
    };

}

#endif //RASTER_TIME_SERIES_SAMPLER_H
