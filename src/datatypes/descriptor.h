
#ifndef RASTER_TIME_SERIES_DESCRIPTOR_H
#define RASTER_TIME_SERIES_DESCRIPTOR_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "datatypes/raster.h"
#include "util/parameters.h"
#include "datatypes/spatial_temporal_reference.h"
#include "datatypes/order.h"

namespace rts {

    /**
     * Base class for descriptors, containing spatial temporal reference and virtual getRaster method.
     */
    class Descriptor {
    public:
        Descriptor(std::function<UniqueRaster(const Descriptor&)> &&getter,
                   const SpatialTemporalReference &totalInfo,
                   const SpatialReference &tileSpatialInfo,
                   const Resolution &tileResolution,
                   Order order,
                   uint32_t tileIndex,
                   uint32_t tileCount,
                   int nodata);
        static std::optional<Descriptor> createNodataDescriptor(SpatialTemporalReference &totalInfo,
                                                                SpatialReference &tileSpatialInfo,
                                                                Resolution &tileResolution,
                                                                Order order,
                                                                uint32_t tileIndex,
                                                                uint32_t tileCount,
                                                                int nodata);

        std::unique_ptr<Raster> getRaster() const;

        /**
         * The order in which the tiles are ordered.
         */
        Order order;

        /**
         * Spatial, temporal, and resolution information about the raster this tile is part of.
         */
        SpatialTemporalReference rasterInfo;

        /**
         * Which tile of the raster this descriptor describes. For spatial ordering it also describes the tile of
         * the raster, not which tile of the the time series is described.
         */
        uint32_t tileIndex;

        /**
         * The number of tiles of this raster.
         */
        uint32_t rasterTileCount;

        /**
         * The spatial coordinates and projection of the described tile. This are the real coordinates of the tile, not
         * just for the valid data of it.
         */
         SpatialReference tileSpatialInfo;

        /**
         * The resolution of the described tile.
         */
        Resolution tileResolution;

        /**
         * The nodata value.
         */
        int nodata;

        /**
         * @return If all the data in this tile is nodata.
         */
        bool isOnlyNodata() const;
        
    private:
        bool _isOnlyNodata;
        std::function<UniqueRaster(const Descriptor&)> getter;
    };

    using OptionalDescriptor = std::optional<Descriptor>;
    using OptionalDescriptorVector = std::vector<std::optional<Descriptor>>;
}

#endif //RASTER_TIME_SERIES_DESCRIPTOR_H
