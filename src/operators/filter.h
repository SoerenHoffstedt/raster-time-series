
#ifndef RASTER_TIME_SERIES_FILTER_H
#define RASTER_TIME_SERIES_FILTER_H

#include "operators/generic_operator.h"

namespace rts {

    class Filter : public GenericOperator {
    public:
        explicit Filter(Json::Value &params);
        UniquePtrTimeSeries createTimeSeries(std::vector<TimeSeries *> &inputs, std::shared_ptr<GenericOperator> op_ptr) override;
        Raster* executeOnRaster(Descriptor *descriptor) override;
    };

}

#endif //RASTER_TIME_SERIES_FILTER_H
