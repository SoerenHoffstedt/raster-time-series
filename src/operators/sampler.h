
#ifndef RASTER_TIME_SERIES_SAMPLER_H
#define RASTER_TIME_SERIES_SAMPLER_H

#include "operators/generic_operator.h"

namespace rts {


    class Sampler : public GenericOperator {
    public:
        Sampler(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        OptionalDescriptor nextDescriptor() override;
        bool supportsOrder(Order order) const override;
    private:
        uint32_t toSkip;
        double lastSendT1;
    };

}

#endif //RASTER_TIME_SERIES_SAMPLER_H
