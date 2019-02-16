
#ifndef RASTER_TIME_SERIES_CACHE_SIMULATOR_H
#define RASTER_TIME_SERIES_CACHE_SIMULATOR_H

#include "operators/generic_operator.h"

namespace rts {

    /**
     * Operator that caches the tile data of a raster but not for tiles from multiple different rasters.
     * It does not change incoming descriptors, except it is changing the getRaster closure.
     * The closure checks if the tile is already present in an internal list and if so it loads it from there.
     * If it is not available the tile data will be loaded and saved in the list.
     * Can be used, for example, before convolution operator to stop loading the same tile data multiple times from disk.
     * It will cache the tile data only for the time of executing the query.
     */
    class RasterCache : public GenericOperator {
    public:
        RasterCache(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, UniqueOperatorVector &&in);
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
