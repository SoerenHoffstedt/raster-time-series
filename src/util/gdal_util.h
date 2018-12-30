
#ifndef RASTER_TIME_SERIES_GDAL_UTIL_H
#define RASTER_TIME_SERIES_GDAL_UTIL_H

#include <mutex>

namespace rts {

    class GDALUtil {
    public:
        static void initGdal();
        static std::string timeToString(time_t time, const std::string &timeFormat);
    private:
        static std::once_flag gdal_init_once;
    };

}

#endif //RASTER_TIME_SERIES_GDAL_UTIL_H
