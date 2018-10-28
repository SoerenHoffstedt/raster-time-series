
#ifndef RASTER_TIME_SERIES_CUMULATIVE_SUM_H
#define RASTER_TIME_SERIES_CUMULATIVE_SUM_H

#include "operators/generic_operator.h"

namespace rts {

    class CumulativeSum : public GenericOperator {
    public:
        CumulativeSum(QueryRectangle qrect, Json::Value &params, const std::vector<GenericOperator *> &in);
        OptionalDescriptor next() override;
        bool supportsOrder(Order order) override;
    private:
        Raster sum;
        SpatialReference lastTileSpat;
        TemporalReference lastTileTemp;
        void setSumRasterZero(const QueryRectangle &rect);
    };

}

#endif //RASTER_TIME_SERIES_CUMULATIVE_SUM_H
