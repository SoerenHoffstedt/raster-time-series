
#ifndef RASTER_TIME_SERIES_CACHE_SIMULATOR_H
#define RASTER_TIME_SERIES_CACHE_SIMULATOR_H

#include "operators/generic_operator.h"

namespace rts {

    class CacheSimulator : public GenericOperator {
    public:
        CacheSimulator(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, UniqueOperatorVector &&in);
        OptionalDescriptor nextDescriptor() override;
        OptionalDescriptor getDescriptor(int tileIndex);
        void initialize() override;
        bool supportsOrder(Order order) const override;
    private:
        OptionalDescriptor createOutput(OptionalDescriptor &input);
        std::vector<UniqueRaster> cache;
        int lastTileIndex;
    };

}

#endif //RASTER_TIME_SERIES_CACHE_SIMULATOR_H
