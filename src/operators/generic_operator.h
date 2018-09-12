
#ifndef RASTER_TIME_SERIES_GENERIC_OPERATOR_H
#define RASTER_TIME_SERIES_GENERIC_OPERATOR_H

#include <json/json.h>
#include <vector>
#include "datatypes/timeseries.h"
#include "datatypes/raster.h"
#include "util/parameters.h"
#include "util/make_unique.h"


namespace rts {

    class TimeSeries;
    class Descriptor;

    class GenericOperator {
    public:
        explicit GenericOperator(Json::Value &params);
        virtual ~GenericOperator() { };
        virtual std::unique_ptr<TimeSeries> createTimeSeries(std::vector<TimeSeries *> &inputs,
                                                             std::shared_ptr<GenericOperator> op_ptr) = 0;
        virtual Raster* executeOnRaster(Descriptor *descriptor) = 0;
        static std::shared_ptr<GenericOperator> getSharedOperator(const std::string &op_name, Json::Value &params);

    protected:
        Json::Value params;
        void checkInputCount(std::vector<TimeSeries*> &inputs, int expected);
    };

}


#endif //RASTER_TIME_SERIES_GENERIC_OPERATOR_H
