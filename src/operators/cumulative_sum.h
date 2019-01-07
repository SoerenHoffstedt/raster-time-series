
#ifndef RASTER_TIME_SERIES_CUMULATIVE_SUM_H
#define RASTER_TIME_SERIES_CUMULATIVE_SUM_H

#include "operators/generic_operator.h"

namespace rts {

    class CumulativeSum : public GenericOperator {
    public:
        CumulativeSum(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        OptionalDescriptor nextDescriptor() override;
        void initialize() override;
        bool supportsOrder(Order order) const override;
    private:
        UniqueRaster sum;
        int lastTileIndex;
        TemporalReference lastTileTemp;
        void setSumRasterZero(const Resolution &res);
    };

}

#endif //RASTER_TIME_SERIES_CUMULATIVE_SUM_H
