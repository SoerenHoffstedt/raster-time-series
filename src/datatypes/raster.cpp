
#include <stdexcept>
#include <iostream>
#include "datatypes/raster.h"
#include "raster.h"


using namespace rts;

Raster::Raster(GDALDataType dataType, int res_x, int res_y)
        : dataType(dataType), res_x(res_x), res_y(res_y), data_length(res_x * res_y) { }


Raster::Raster(GDALDataType dataType, Resolution res)
        : dataType(dataType), res_x(res.res_x), res_y(res.res_y), data_length(res.res_x * res.res_y) { }

/*Raster::Raster(const Raster &other)
        : data_length(other.data_length),
          res_x(other.res_x),
          res_y(other.res_y),
          data(new int[data_length])
{
    std::copy(other.data, other.data + other.data_length, data);
}

Raster& Raster::operator=(const Raster &other) {
    delete[] data;
    data_length = other.data_length;
    res_x = other.res_x;
    res_y = other.res_y;
    data = new int[data_length];
    std::copy(other.data, other.data + other.data_length, data);
    return *this;
}

Raster::Raster(Raster &&other) noexcept
        : data(other.data),
          data_length(other.data_length),
          res_x(other.res_x),
          res_y(other.res_y)
{
    other.data = nullptr;
    other.data_length = 0;
    other.res_x = 0;
    other.res_y = 0;
}

Raster &Raster::operator=(Raster &&other) noexcept {
    data = other.data;
    data_length = other.data_length;
    res_x = other.res_x;
    res_y = other.res_y;

    other.data = nullptr;
    other.data_length = 0;
    other.res_x = 0;
    other.res_y = 0;

    return *this;
}
 */

int Raster::getDataLength() const {
    return data_length;
}

GDALDataType Raster::getDataType() const {
    return dataType;
}

Resolution Raster::getResolution() const {
    return Resolution(res_x, res_y);
}

template<>
void TypedRaster<unsigned char>::print() const {
    for(int y = 0; y < res_y && y < MAX_PRINT_SIZE; y++){
        for(int x = 0; x < res_x && x < MAX_PRINT_SIZE; x++){
            std::cout << static_cast<int>(data[x + y * res_x]) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}
