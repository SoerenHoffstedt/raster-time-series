
#ifndef RASTER_TIME_SERIES_AGGREGATOR_H
#define RASTER_TIME_SERIES_AGGREGATOR_H

#include "generic_operator.h"
#include "util/gdal_util.h"
#include "util/time_interval.h"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace rts {

    /***
     * Allows aggregating in time intervals. If no time interval is provided in the parameters the whole time series is aggregated.
     * The time interval starts from qrect t1 and increases by the passed time interval
     *
     * Parameters:
     *  - custom_data_type: [Byte, UInt16, Int16, UInt32, Int32, Float32, Float64], when not provided data type of input tiles is used.
     *  - time_interval:
     *      - unit: [Year,Month,Day,Hour,Seconds]
     *      - value: number
     *
     */
    class Aggregator : public GenericOperator {
    public:
        Aggregator(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        OptionalDescriptor nextDescriptor() override;
        bool supportsOrder(Order order) const override;
    private:
        OptionalDescriptor nextDesc;
        GDALDataType customDataType;

        bool hasTimeInterval;
        TimeInterval interval;
        boost::posix_time::ptime currTime;
        int lastTileIndex;

        double getNextTimeBorder();
    };

}

#endif //RASTER_TIME_SERIES_AGGREGATOR_H
