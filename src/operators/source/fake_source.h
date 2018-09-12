
#ifndef RASTER_TIME_SERIES_FAKE_SOURCE_H
#define RASTER_TIME_SERIES_FAKE_SOURCE_H

#include "operators/generic_operator.h"

namespace rts {

    class FakeSource : public GenericOperator {
    public:
        explicit FakeSource(Json::Value &params);
        std::unique_ptr<TimeSeries> createTimeSeries(std::vector<TimeSeries *> &inputs,
                                                     std::shared_ptr<GenericOperator> op_ptr) override;
        Raster* executeOnRaster(Descriptor *descriptor) override;
    private:
        Json::Value loadDatasetJson(std::string name);
    };

}

#endif //RASTER_TIME_SERIES_FAKE_SOURCE_H
