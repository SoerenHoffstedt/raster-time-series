
#ifndef RASTER_TIME_SERIES_TEMPORAL_OVERLAP_H
#define RASTER_TIME_SERIES_TEMPORAL_OVERLAP_H

#include "operators/generic_operator.h"
#include <deque>

namespace rts {

    /**
     * Takes two time series as inputs and returns where rasters of both time series overlap temporally the mean of these rasters.
     * TODO: for now only returns values of one the rasters, taking both into account needs the same tactics applied a second time.
     */
    class TemporalOverlap : public GenericOperator {
    public:
        TemporalOverlap(QueryRectangle qrect, Json::Value &params, const std::vector<GenericOperator*> &in);
        OptionalDescriptor next() override;
        bool supportsOrder(Order order) override;
    private:
        TemporalReference input1Time;
        TemporalReference input2Time;
        std::vector<OptionalDescriptor> descriptorCache1;
        std::vector<OptionalDescriptor> descriptorCache2;
        bool lastTileOfRaster1;
        bool lastTileOfRaster2;
        bool loadRasterFromCache1;
        bool loadRasterFromCache2;
    };

}

#endif //RASTER_TIME_SERIES_TEMPORAL_OVERLAP_H
