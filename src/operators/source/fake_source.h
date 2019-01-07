
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
        explicit FakeSource(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, UniqueOperatorVector &&in);
        OptionalDescriptor nextDescriptor() override;
        void initialize() override;
        bool supportsOrder(Order o) const override;
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
        SpatialReference extent;

        /***
         * if true the data thats written into the tile is the raster index, if false the pixel will be sum of its index (x+y)
         */
        bool fill_with_index;

        ///
        /// \return returns true if end of temporal iteration is reached.
        bool increaseTemporal();
        ///
        /// \return returns true if end of spatial iteration is reached.
        bool increaseSpatial();
    };

}

#endif //RASTER_TIME_SERIES_FAKE_SOURCE_H
