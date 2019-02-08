
#ifndef RASTER_TIME_SERIES_SAMPLER_H
#define RASTER_TIME_SERIES_SAMPLER_H

#include "operators/generic_operator.h"

namespace rts {

    /**
     * Operator for skipping rasters from a time series.
     * It does not matter if the time series is spatially or temporally ordered,
     * in both cases the same complete rasters will be skipped.
     *
     * The operator is not sampling at the moment, because it is not known, how many rasters a time series has.
     * Therefore it just skipping rasters at the moment.
     *
     * Parameters:
     *  - to_skip: how many rasters must be skipped at once. [uint]
     *  - to_keep: how many rasters will be returned after a skip. [uint]
     */
    class Sampler : public GenericOperator {
    public:
        Sampler(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        OptionalDescriptor nextDescriptor() override;
        OptionalDescriptor getDescriptor(int tileIndex) override;
        void initialize() override;
        bool supportsOrder(Order order) const override;
    private:
        const uint32_t toSkip;
        const uint32_t toReturn;
        uint32_t returningCount;
        uint32_t lastSendTileIndex;
    };

}

#endif //RASTER_TIME_SERIES_SAMPLER_H
