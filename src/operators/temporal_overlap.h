
#ifndef RASTER_TIME_SERIES_TEMPORAL_OVERLAP_H
#define RASTER_TIME_SERIES_TEMPORAL_OVERLAP_H

#include "operators/generic_operator.h"
#include "util/expression.h"

namespace rts {

    /**
     * Takes two time series as inputs and returns rasters for times where rasters of both time series overlap.
     * The returned raster is defined by an expression.
     *
     * Params:
     *  - expression: String defining a valid expression for the Expression class.
     *
     */
    class TemporalOverlap : public GenericOperator {
    public:
        TemporalOverlap(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        OptionalDescriptor nextDescriptor() override;
        void initialize() override;
        bool supportsOrder(Order order) const override;
    private:
        TemporalReference input1Time;
        TemporalReference input2Time;
        std::vector<OptionalDescriptor> descriptorCache1;
        std::vector<OptionalDescriptor> descriptorCache2;
        bool lastTileOfRaster1;
        bool lastTileOfRaster2;
        bool loadRasterFromCache1;
        bool loadRasterFromCache2;
        Expression expression;
    };

}

#endif //RASTER_TIME_SERIES_TEMPORAL_OVERLAP_H
