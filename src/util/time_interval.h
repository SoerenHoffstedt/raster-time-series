
#ifndef RASTER_TIME_SERIES_TIME_INTERVAL_H
#define RASTER_TIME_SERIES_TIME_INTERVAL_H

#include <boost/date_time/posix_time/posix_time.hpp>
#include <json/json.h>


namespace rts {

    /**
    * Enum representing the unit of a date.
    */
    enum class TimeUnit {
        Year 	= 0,
        Month 	= 1,
        Day 	= 2,
        Hour 	= 3,
        Minute  = 4,
        Second  = 5
    };

    /**
     * Class representing time intervals by a unit (year, month, day, ...) and a length. E.g. with the unit month
     * and the length 3 the time interval represents time steps of three months.
     */
    class TimeInterval {
    public:
        TimeInterval();
        TimeInterval(TimeUnit unit, uint32_t length);
        explicit TimeInterval(const Json::Value &json);

        /**
         * The time unit of this interval, e.g. Month or Hour.
         */
        TimeUnit unit;

        /**
         * How many units this interval covers.
         */
        uint32_t length;

        /**
         * Increase the passed time by the interval described by this object.
         * @param time Reference to the time that is to be increased by this interval.
         * @param times Factor to allow increasing multiple times with one call. Standard is 1.
         */
        void increase(boost::posix_time::ptime &time, uint32_t times = 1) const;

        /**
         * Decrease the passed time by the interval described by this object.
         * @param time Reference to the time that is to be decreased by this interval.
         * @param times Factor to allow decreasing multiple times with one call. Standard is 1.
         */
        void decrease(boost::posix_time::ptime &time, uint32_t times = 1) const;
    };

}

#endif //RASTER_TIME_SERIES_TIME_INTERVAL_H
