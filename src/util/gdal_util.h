
#ifndef RASTER_TIME_SERIES_GDAL_UTIL_H
#define RASTER_TIME_SERIES_GDAL_UTIL_H

#include <mutex>

namespace rts {

    /**
    * enum representing the unit of a date that is snapped to
    */
    enum class TimeUnit {
        Year 	= 0,
        Month 	= 1,
        Day 	= 2,
        Hour 	= 3,
        Minute  = 4,
        Second  = 5
    };

    class GDALUtil {
    public:
        static void initGdal();
    private:
        static std::once_flag gdal_init_once;
    };

}

#endif //RASTER_TIME_SERIES_GDAL_UTIL_H
