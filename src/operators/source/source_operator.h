
#ifndef RASTER_TIME_SERIES_SOURCE_OPERATOR_H
#define RASTER_TIME_SERIES_SOURCE_OPERATOR_H

#include "operators/generic_operator.h"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace rts {

    /**
     * Implements common functionality for source operators like increasing temporally and spatially between
     * nextDescriptor calls.
     * Source operators must implement only the createDescriptor method to return tiles for either nextDescriptor()
     * or getDescriptor() but if needed they can overwrite these methods too.
     */
    class SourceOperator : public GenericOperator {
    public:
        SourceOperator(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        /**
         * Method for returning a descriptor for a specific time and space. Will be used by nextDescriptor()
         * as well as getDescriptor().
         * @param time The time identifying the raster to be used.
         * @param pixelStartX pixel positions from where to return the tile.
         * @param pixelStartY pixel positions from where to return the tile.
         * @return Descriptor for the tile.
         */
        virtual OptionalDescriptor createDescriptor(double time, int pixelStartX, int pixelStartY, int tileIndex) = 0;
        OptionalDescriptor nextDescriptor() override;
        OptionalDescriptor getDescriptor(int tileIndex) override;
    protected:
        /**
         * Increases the spatial position of the pixelState
         * @return true when increasing the spatial tile went over the end of the current raster.
         */
        virtual bool increaseSpatially();

        /**
         * Increases the time to the next rasters starting time in the time series.
         * @return true when increasing the time to the next raster went over the end of the time series.
         */
        virtual bool increaseTemporally();

        /**
         * In temopral order, skipCurrentRaster will set the spatial and temporal state variables to the start
         * of the next raster in temporal order. It will skip skipCount amount of rasters.
         * In spatial order, it will skip tiles of rasters. If the last raster of that tile is skipped by skipCount skips
         * it will reset to the first raster of the next tile but it does not skip any rasters of the next tile.
         */
        void skipCurrentRaster(const uint32_t skipCount) override;

        /**
         * In spatial order, skipCurrentTile will set the spatial and temporal state variables to the start of
         * the next tile and reset the the temporal state to the first raster of the output time series.
         * It will skip as many tiles as skipCount is set to.
         * In temporal order, it skips single tiles of the raster. When skipCount is so big that the end of
         * the raster is reached, it stays at the beginning of the next raster and does not skip tiles of it.
         */
        void skipCurrentTile(const uint32_t skipCount) override;

        /**
         * Calculates the pixel start position of a tile based on its index.
         * Will be used when getDescriptor(int index) is implemented.
         */
        Resolution tileIndexToStartPixel(int tileIndex);

        /**
         * If set true the operator will increase dimension on the next nextDescriptor() call.
         * Has to be false if the start state for the next descriptor is already set,
         * e.g. by calling intialize() or skippCurrentRaster()/-Tile().
         *
         */
        bool increaseDimensions;
    private:
        /**
         * The beginning pixel X position of the last returned tile.
         * Can be negative when the query bounding box does not align to tile positions.
         */
        int pixelStateX;
        /**
         * The beginning pixel Y position of the last returned tile.
         * Can be negative when the query bounding box does not align to tile positions.
         */
        int pixelStateY;
    protected:
        /**
         * Two-dimensional count of the number of tiles in the returned rasters.
         */
        Resolution tileCount;
        /**
         * Start time of the last returned tile.
         */
        double currTime;
        /**
         * Index of the last returned tile in its raster.
         */
        int currTileIndex;
        /**
         * Index of the last returned raster of the returned time series.
         */
        int currRasterIndex;
        /**
         * The pixel position of the start of query bounding box in the extent of source raster,
         * based on the query resolution not the source rasters resolution.
         */
        Resolution rasterWorldPixelStart;
        /**
         * Contains pixel scale information of the outgoing rasters.
         */
        Scale scale;
        /**
         * The coordinate origin of the source raster.
         */
        Origin origin;
        /**
         * The start time of the dataset used by the operator.
         */
        double datasetStartTime;
        /**
         * The end time of the dataset used by the operator.
         */
        double datasetEndTime;
        /**
         * Increase the currTime variable to the time the next raster starts.
         */
        virtual void increaseCurrentTime() = 0;
        /**
         * Get the end time of the validity of the current raster.
         * @return The exclusive end time of the raster's temporal validity.
         */
        virtual double getCurrentTimeEnd() const = 0;
        /**
         * Sets internal currTime variable to the start of the first raster that is part of the query.
         */
        virtual void setCurrTimeToFirstRaster();

    };

}

#endif //RASTER_TIME_SERIES_SOURCE_OPERATOR_H
