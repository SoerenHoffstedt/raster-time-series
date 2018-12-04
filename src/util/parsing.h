
#ifndef RASTER_TIME_SERIES_PARSING_H
#define RASTER_TIME_SERIES_PARSING_H

#include <gdal.h>

namespace rts {

    class Parsing {
    public:
        static GDALDataType parseDataType(const std::string &input);

    };

}


#endif //RASTER_TIME_SERIES_PARSING_H
