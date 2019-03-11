
#ifndef RASTER_TIME_SERIES_SOURCE_BACKEND_H
#define RASTER_TIME_SERIES_SOURCE_BACKEND_H

#include "datatypes/descriptor.h"
#include <json/json.h>
#include "datatypes/spatial_temporal_reference.h"

namespace rts {

    class SourceBackend {
    public:
        SourceBackend(const QueryRectangle &qrect, const Json::Value &params);

        virtual void initialize() = 0;
        virtual bool supportsOrder(Order o) const = 0;

        /**
         * Method for returning a descriptor for a specific time and space. Will be used by nextDescriptor()
         * as well as getDescriptor().
         * @param time The time identifying the raster to be used.
         * @param pixelStartX pixel positions from where to return the tile.
         * @param pixelStartY pixel positions from where to return the tile.
         * @return Descriptor for the tile.
         */
        virtual OptionalDescriptor createDescriptor(double time, int pixelStartX, int pixelStartY, int tileIndex, const Resolution &rasterWorldPixelStart, const Scale &scale, const Origin &origin, const Resolution &tileCount) = 0;

        virtual void beforeSpatialIncrease();
        virtual void beforeTemporalIncrease();
        /**
         * Increase the currTime variable to the time the next raster starts.
         */
        virtual void increaseCurrentTime(double &currTime) = 0;

        /**
         * Get the end time of the validity of the current raster.
         * @return The exclusive end time of the raster's temporal validity.
         */
        virtual double getCurrentTimeEnd(double currTime) const = 0;

        /**
         * The origin of the raster time series must be provided by the backend.
         */
        virtual Origin getOrigin() const = 0;

        /**
         * The start time of the dataset used by the operator.
         */
        double datasetStartTime;
        /**
         * The end time of the dataset used by the operator.
         */
        double datasetEndTime;

    protected:
        const Json::Value &params;
        const QueryRectangle &qrect;
    };

}

#endif //RASTER_TIME_SERIES_SOURCE_BACKEND_H
