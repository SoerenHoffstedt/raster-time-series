
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
        OptionalDescriptor next() override;
        bool supportsOrder(Order o) override;
    private:
        Json::Value loadDatasetJson(std::string name);
        Json::Value dataset_json;
        int rasterIndex;
        int tileIndex;
        int raster_count;
        int nodata;
        double time_start;
        double time_curr;
        double time_duration;
        Resolution tile_res;
        int state_x;
        int state_y;

        ///
        /// \return returns true if end of temporal iteration is reached.
        bool increaseTemporal();
        ///
        /// \return returns true if end of spatial iteration is reached.
        bool increaseSpatial();
        static SpatialReference getCoordsForTile(const SpatialReference &totalCoords, const Resolution &totalRes, const Resolution &tileRes, const Resolution &startOfTile);
    };

}

#endif //RASTER_TIME_SERIES_FAKE_SOURCE_H
