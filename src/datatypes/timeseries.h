
#ifndef RASTER_TIME_SERIES_TIMESERIES_H
#define RASTER_TIME_SERIES_TIMESERIES_H

#include <vector>
#include "datatypes/descriptor.h"

namespace rts {
    class Descriptor;

    class TimeSeries {
    public:
        using iterator = std::vector<Descriptor>::iterator;
        iterator begin();
        iterator end();

        void add(Descriptor descriptor);
        void move(Descriptor &&descriptor);

        unsigned long size() { return descriptors.size(); }
        Descriptor& get(unsigned long i) { return descriptors[i]; }

        /**
         *
         * @param time
         * @return a time series with raster valid at provided time
         */
        std::unique_ptr<TimeSeries> atTime(double time);
        /**
         *
         * @param time_start
         * @param time_end
         * @return a time series containing all rasters of this series valid between the two provided times
         */
        std::unique_ptr<TimeSeries> between(double time_start, double time_end);

        /**
         *
         * @param sample_count
         * @return
         */
        std::unique_ptr<TimeSeries> sample(unsigned int sample_count);

    private:
        std::vector<Descriptor> descriptors;
    };

    using UniquePtrTimeSeries = std::unique_ptr<TimeSeries>;

}

#endif //RASTER_TIME_SERIES_TIMESERIES_H
