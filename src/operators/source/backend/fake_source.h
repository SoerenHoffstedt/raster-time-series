
#ifndef RASTER_TIME_SERIES_FAKE_SOURCE_H
#define RASTER_TIME_SERIES_FAKE_SOURCE_H

#include "operators/source/source_operator.h"

namespace rts {


    /**
     * Source operator for creating test data used to test the workflow.
     * It is based on datasets stored on disc providing resolution and time info. Each raster has its index as data.
     */

    class FakeSource : public SourceBackend {
    public:
        explicit FakeSource(const QueryRectangle &qrect, const Json::Value &params);
        OptionalDescriptor createDescriptor(double time, int pixelStartX, int pixelStartY, int tileIndex, const Resolution &rasterWorldPixelStart, const Scale &scale, const Origin &origin, const Resolution &tileCount) override;
        void initialize() override;
        bool supportsOrder(Order o) const override;
        Origin getOrigin() const override;
    private:
        Json::Value loadDatasetJson(std::string name);
        int rasterCount;
        int currRasterIndex;
        double lastTime;
        double nodata;
        GDALDataType dataType;
        double timeDuration;
        SpatialReference extent;
        Origin origin;

        /***
         * if true the data thats written into the tile is the raster index, if false the pixel will be sum of its index (x+y)
         */
        bool fill_with_index;

        /**
         * Increase the currTime variable to the time the next raster starts.
         */
        void increaseCurrentTime(double &currTime) override;
        /**
         * Get the end time of the validity of the current raster.
         * @return
         */
        double getCurrentTimeEnd(double currTime) const override;

    };

}

#endif //RASTER_TIME_SERIES_FAKE_SOURCE_H
