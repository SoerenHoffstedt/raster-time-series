
#ifndef RASTER_TIME_SERIES_DESCRIPTOR_H
#define RASTER_TIME_SERIES_DESCRIPTOR_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <gdal.h>
#include <boost/optional.hpp>
#include "util/make_optional.h"
#include "datatypes/raster.h"
#include "datatypes/spatial_temporal_reference.h"
#include "datatypes/order.h"

namespace rts {

    class Descriptor;

    /**
    * A class of all the metadata saved in a Descriptor. The Descriptor class inherits DescriptorInfo.
    * This extra class is used to clean up the passing of arguments from an input descriptor to an
    * output descriptor in operators.
    * Because many information of an input descriptor will be the same for the output, the operator has to create
    * fields to save all of them before moving the input descriptor into the closure.
    * This class contains all the meta information of a tile for the Descriptor, excluding the getter
    * (that will be different in the output).
    * Therefore all information can be copied at once, without additionally copying the getRaster closure.
    * The Descriptor provides a constructor that takes the getter function and a DescriptorArguments object.
    */
    class DescriptorInfo {
    public:
        DescriptorInfo(const SpatialTemporalReference &totalInfo,
                            const SpatialReference &tileSpatialInfo,
                            const Resolution &tileResolution,
                            Order order,
                            uint32_t tileIndex,
                            Resolution rasterTileCountDimensional,
                            double nodata,
                            GDALDataType dataType);
        explicit DescriptorInfo(const boost::optional<Descriptor> &desc);
        DescriptorInfo& operator=(const boost::optional<Descriptor> &desc);
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
         * The number of tiles of the complete raster.
         */
        uint32_t rasterTileCount;

        /**
         * The number of tiles of the complete raster in each dimension. rasterTileCount is
         * the product of both dimensional fields.
         */
        Resolution rasterTileCountDimensional;

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

    protected:
        bool _isOnlyNodata;
    };

    /**
     * Core class for descriptors, containing metadata about a tile through inheritance of DescriptorInfo and
     * a std::function that is used to load the raster in getRaster().
     */
    class Descriptor : public DescriptorInfo {
    public:
        Descriptor(std::function<UniqueRaster(const Descriptor&)> &&getter,
                   const SpatialTemporalReference &totalInfo,
                   const SpatialReference &tileSpatialInfo,
                   const Resolution &tileResolution,
                   Order order,
                   uint32_t tileIndex,
                   Resolution rasterTileCountDimensional,
                   double nodata,
                   GDALDataType dataType);

        Descriptor(std::function<UniqueRaster(const Descriptor&)> &&getter,
                   const DescriptorInfo &args);

        Descriptor(const Descriptor &other) = default;
        Descriptor& operator=(const Descriptor &other) = default;

        /**
         * Creates a descriptor that is only nodata. Adds a getter that fills the raster only with nodata and saves that work.
         */
        static boost::optional<Descriptor> createNodataDescriptor(SpatialTemporalReference &totalInfo,
                                                                SpatialReference &tileSpatialInfo,
                                                                Resolution &tileResolution,
                                                                Order order,
                                                                uint32_t tileIndex,
                                                                Resolution rasterTileCountDimensional,
                                                                double nodata,
                                                                GDALDataType dataType);

        /**
         * Call the saved getter closure to create the raster/tile described by this descriptor. Called multiple times
         * it would always create a new unique raster.
         * @return A unique pointer to the raster described by this descriptor.
         */
        std::unique_ptr<Raster> getRaster() const;

    private:
        /**
         * Member variable to save the getRaster closure created by the operator that created this descriptor.
         * The std::function saves the captured values of the closure in dynamic memory. Therefore moving a descriptor
         * is preferred when possible.
         */
        std::function<UniqueRaster(const Descriptor&)> getter;
    };

    using OptionalDescriptor = boost::optional<Descriptor>;
    using OptionalDescriptorVector = std::vector<boost::optional<Descriptor>>;
}

#endif //RASTER_TIME_SERIES_DESCRIPTOR_H
