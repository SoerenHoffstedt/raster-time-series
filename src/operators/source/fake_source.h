
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
        explicit FakeSource(Json::Value &params);
        UniqueDescriptor next() override;
    private:
        Json::Value loadDatasetJson(std::string name);
        Json::Value dataset_json;
        int index;
        int raster_count;
        double time_start;
        double time_duration;
        int res_x;
        int res_y;
    };

}

#endif //RASTER_TIME_SERIES_FAKE_SOURCE_H
