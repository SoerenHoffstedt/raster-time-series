
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
        //Descriptor(std::function<UniqueRaster (const Descriptor&)> getter, TemporalReference temp_ref_total, SpatialReference spat_ref_total, Resolution res_total, TemporalReference temp_ref_tile, SpatialReference spat_ref_tile, Resolution res_tile, Order order);
        Descriptor(std::function<UniqueRaster (const Descriptor&)> &&getter, QueryRectangle &qrect_total, QueryRectangle &qrect_tile);
        Descriptor(std::function<UniqueRaster (const Descriptor&)> &&getter, QueryRectangle &&qrect_total, QueryRectangle &&qrect_tile);
        Descriptor(std::function<UniqueRaster (const Descriptor&)> &&getter, QueryRectangle &qrect_total, QueryRectangle &&qrect_tile);
        Descriptor(std::function<UniqueRaster (const Descriptor&)> &&getter, QueryRectangle &&qrect_total, QueryRectangle &qrect_tile);

        std::unique_ptr<Raster> getRaster() const;
        QueryRectangle tileInfo;
        QueryRectangle totalInfo;
        /**
         *
         * @return The total number of tiles of the raster / tile-time-series the current tile belongs to
         */
        uint64_t tilesOfRaster() const;
    private:
        std::function<UniqueRaster(const Descriptor&)> getter;
    };

    using OptionalDescriptor = std::optional<Descriptor>;
}

#endif //RASTER_TIME_SERIES_DESCRIPTOR_H
