
#ifndef RASTER_TIME_SERIES_FAKE_SOURCE_H
#define RASTER_TIME_SERIES_FAKE_SOURCE_H

#include "operators/generic_operator.h"

namespace rts {

    /**
     * Source operator for creating test data used to test the workflow.
     * It is based on datasets stored on disc providing resolution and time info. Each raster has its index as data.
     */
    class FakeSource : public GenericOperator {
    public:
        explicit FakeSource(QueryRectangle qrect, Json::Value &params);
        OptionalDescriptor nextDescriptor() override;
        bool supportsOrder(Order o) override;
    private:
        Json::Value loadDatasetJson(std::string name);
        Json::Value dataset_json;
        int rasterIndex;
        int tileIndex;
        int raster_count;
        double nodata;
        GDALDataType dataType;
        double time_start;
        double time_curr;
        double time_duration;
        Resolution tile_res;
        int state_x;
        int state_y;
        Resolution rasterWorldPixelStart;
        uint32_t tileCount;

        ///
        /// \return returns true if end of temporal iteration is reached.
        bool increaseTemporal();
        ///
        /// \return returns true if end of spatial iteration is reached.
        bool increaseSpatial();
    };

}

#endif //RASTER_TIME_SERIES_FAKE_SOURCE_H
