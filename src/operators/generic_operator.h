
#ifndef RASTER_TIME_SERIES_GENERIC_OPERATOR_H
#define RASTER_TIME_SERIES_GENERIC_OPERATOR_H

#include <json/json.h>
#include <boost/optional.hpp>
#include <vector>
#include "datatypes/raster.h"
#include "util/parameters.h"
#include "util/make_unique.h"
#include "datatypes/descriptor.h"
#include "datatypes/timeseries_iterator.h"

namespace rts {

    class Descriptor;
    class TimeSeriesIterator;

    class GenericOperator {
    public:
        explicit GenericOperator(QueryRectangle qrect, Json::Value &params, const std::vector<GenericOperator*> &in);
        explicit GenericOperator(QueryRectangle qrect, Json::Value &params);
        virtual ~GenericOperator() = default;
        virtual UniqueDescriptor next() = 0;
        virtual bool supportsOrder(Order order) = 0;
        TimeSeriesIterator begin();
        TimeSeriesIterator end();

    protected:
        QueryRectangle qrect;
        Json::Value params;
        std::vector<GenericOperator*> input_operators;
        /**
         * Throws an exception if the size of input_operators differs from expected.
         * @param expected the amount of expected input operators.
         */
        void checkInputCount(int expected);
    };

    class OperatorUtil {
    public:
        /**
         * Skips all the tiles belonging to the raster of the passed Descriptor (same temporal information as currentDesc).
         * @param op The operator that provides the Descriptors.
         * @param currentDesc All the other tiles of the raster this Descriptor belongs to are skipped.
         * @return The first Descriptor of the next raster.
         */
        static UniqueDescriptor skipCurrentTemporal(GenericOperator &op, UniqueDescriptor currentDesc);
        /**
         * Skips all the tiles of the same spatial coordinates as the passed Descriptor (currentDesc).
         * @param op The operator that provides the Descriptors.
         * @param currentDesc All the other tiles with the same spatial coordinates as this Descriptor are skipped.
         * @return The first Descriptor of the next tile.
         */
        static UniqueDescriptor skipCurrentSpatial(GenericOperator &op, UniqueDescriptor currentDesc);
        /**
         * Skips all tiles that are of the same first dimension as the passed Descriptor.
         * If the order of the passed Descriptor is TemporalSpatial skipCurrentTemporal is called
         * and if the order is SpatialTemporal skipCurrentSpatial is called.
         * @param op The operator that provides the Descriptors.
         * @param currentDesc The descriptor whose current dimension should be skipped.
         * @return The first Descriptor that does not belong to the skipped dimension.
         */
        static UniqueDescriptor skipCurrentDimension(GenericOperator &op, UniqueDescriptor currentDesc);
    };

}


#endif //RASTER_TIME_SERIES_GENERIC_OPERATOR_H
