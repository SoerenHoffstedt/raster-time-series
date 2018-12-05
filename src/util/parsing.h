
#ifndef RASTER_TIME_SERIES_PARSING_H
#define RASTER_TIME_SERIES_PARSING_H

#include <gdal.h>
#include "gdal_util.h"
#include "util/time_interval.h"

namespace rts {

    class Parsing {
    public:
        static GDALDataType parseDataType(const std::string &input);
        static TimeUnit parseTimeUnit(const std::string &input);

    };

}


#endif //RASTER_TIME_SERIES_PARSING_H
