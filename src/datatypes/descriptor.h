
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
        Descriptor(std::function<UniqueRaster(const Descriptor&)> &&getter, SpatialTemporalReference &totalInfo, Resolution &tileResolution, Order order, uint32_t tileIndex, int nodata);
        Descriptor(std::function<UniqueRaster (const Descriptor&)> &&getter, SpatialTemporalReference &&totalInfo, Resolution &&tileResolution, Order order, uint32_t tileIndex, int nodata);
        Descriptor(std::function<UniqueRaster (const Descriptor&)> &&getter, SpatialTemporalReference &totalInfo, Resolution &&tileResolution, Order order, uint32_t tileIndex, int nodata);
        Descriptor(std::function<UniqueRaster (const Descriptor&)> &&getter, SpatialTemporalReference &&totalInfo, Resolution &tileResolution, Order order, uint32_t tileIndex, int nodata);

        std::unique_ptr<Raster> getRaster() const;

        /**
         *
         */
        Order order;

        /**
         *
         */
        SpatialTemporalReference rasterInfo;

        /**
         * Which tile of the raster this descriptor describes. For spatial ordering it also describes the tile of
         * the raster, not which tile of the the time series is described.
         */
        uint32_t tileIndex;

        /**
         *
         */
        Resolution tileResolution;

        /**
         *
         */
        int nodata;

        /**
         * @return The total number of tiles of the raster / tile-time-series the current tile belongs to
         */
        uint64_t tilesOfRaster() const;

        /**
         *
         * @return The coordinates of the described tile.
         */
        SpatialReference calculateCoordinatesOfTile() const;
    private:
        std::function<UniqueRaster(const Descriptor&)> getter;
    };

    using OptionalDescriptor = std::optional<Descriptor>;
}

#endif //RASTER_TIME_SERIES_DESCRIPTOR_H
