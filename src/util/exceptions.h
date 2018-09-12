
#ifndef RASTER_TIME_SERIES_EXCEPTIONS_H
#define RASTER_TIME_SERIES_EXCEPTIONS_H

#include <stdexcept>

class ArgumentException : public std::runtime_error {
public:
    ArgumentException(const std::string &msg) : std::runtime_error(msg) { }


};

#endif //RASTER_TIME_SERIES_EXCEPTIONS_H
