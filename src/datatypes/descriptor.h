
#ifndef RASTER_TIME_SERIES_DESCRIPTOR_H
#define RASTER_TIME_SERIES_DESCRIPTOR_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <gdal.h>
#include "datatypes/raster.h"
#include "util/parameters.h"
#include "datatypes/spatial_temporal_reference.h"
#include "datatypes/order.h"

namespace rts {

    class Descriptor;

    /**
    * A class to clean up the passing of arguments from an input descriptor to an output descriptor in operators.
    * Because many arguments of an input descriptor will be the same in the output, the operator has to create
    * fields to save all of them before moving the input descriptor into the closure.
    * This class is contains all the meta information of a tile for the Descriptor, excluding the getter
    * (that will be different in the output).
    * The actual Descriptor class inherits this class, so the data fields are actually the same.
    * The Descriptor provides a constructor that takes the getter function and a DescriptorArguments object.
    */
    class DescriptorInfo {
    public:
        DescriptorInfo(const SpatialTemporalReference &totalInfo,
                            const SpatialReference &tileSpatialInfo,
                            const Resolution &tileResolution,
                            Order order,
                            uint32_t tileIndex,
                            uint32_t tileCount,
                            double nodata,
                            GDALDataType dataType);
        explicit DescriptorInfo(const std::optional<Descriptor> &desc);
        DescriptorInfo& operator=(const std::optional<Descriptor> &desc);
        DescriptorInfo(const DescriptorInfo &desc) = default;
        DescriptorInfo& operator=(const DescriptorInfo &desc) = default;

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
         * just for the valid data of it. So it can exceed the spatial reference of the whole raster and
         * the projections extent.
         */
        SpatialReference tileSpatialInfo;

        /**
         * The resolution of the described tile.
         */
        Resolution tileResolution;

        /**
         * The nodata value. A double can contain values for all different dataTypes.
         */
        double nodata;

        /**
         * The data type of the tile.
         */
        GDALDataType dataType;

        /**
         * @return If all the data in this tile is nodata.
         */
        bool isOnlyNodata() const;

        Resolution getDimensionalTileCount() const;

    protected:
        bool _isOnlyNodata;
    };

    /**
     * Core class for descriptors, containing meta data about a tile (see DescriptorArguments) and
     * an std::function that is used to load the raster in getRaster().
     */
    class Descriptor : public DescriptorInfo {
    public:
        Descriptor(std::function<UniqueRaster(const Descriptor&)> &&getter,
                   const SpatialTemporalReference &totalInfo,
                   const SpatialReference &tileSpatialInfo,
                   const Resolution &tileResolution,
                   Order order,
                   uint32_t tileIndex,
                   uint32_t tileCount,
                   double nodata,
                   GDALDataType dataType);

        Descriptor(std::function<UniqueRaster(const Descriptor&)> &&getter,
                   const DescriptorInfo &args);

        Descriptor(const Descriptor &other) = default;
        Descriptor& operator=(const Descriptor &other) = default;

        static std::optional<Descriptor> createNodataDescriptor(SpatialTemporalReference &totalInfo,
                                                                SpatialReference &tileSpatialInfo,
                                                                Resolution &tileResolution,
                                                                Order order,
                                                                uint32_t tileIndex,
                                                                uint32_t tileCount,
                                                                double nodata,
                                                                GDALDataType dataType);

        std::unique_ptr<Raster> getRaster() const;

    private:
        std::function<UniqueRaster(const Descriptor&)> getter;
    };

    using OptionalDescriptor = std::optional<Descriptor>;
    using OptionalDescriptorVector = std::vector<std::optional<Descriptor>>;
}

#endif //RASTER_TIME_SERIES_DESCRIPTOR_H
