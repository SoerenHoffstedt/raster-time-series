
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
     * and the length 3 the time interval represents steps of three months.
     */
    class TimeInterval {
    public:
        TimeInterval();
        TimeInterval(TimeUnit unit, uint32_t length);
        explicit TimeInterval(const Json::Value &json);
        TimeUnit unit;
        uint32_t length;
        void increase(boost::posix_time::ptime &time) const;
    };

}

#endif //RASTER_TIME_SERIES_TIME_INTERVAL_H
