
#ifndef RASTER_TIME_SERIES_SAMPLER_H
#define RASTER_TIME_SERIES_SAMPLER_H

#include "operators/generic_operator.h"

namespace rts {


    class Sampler : public GenericOperator {
    public:
        Sampler(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        OptionalDescriptor nextDescriptor() override;
        OptionalDescriptor getDescriptor(int tileIndex) override;
        void initialize() override;
        bool supportsOrder(Order order) const override;
    private:

        uint32_t toSkip;
        double lastSendT1;
    };

}

#endif //RASTER_TIME_SERIES_SAMPLER_H
