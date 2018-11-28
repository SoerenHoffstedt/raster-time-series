
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

std::unique_ptr<Raster> Raster::createRaster(GDALDataType dataType, int res_x, int res_y) {
    Raster *ptr = nullptr;

    switch(dataType){
        case GDT_Byte:
            ptr = static_cast<Raster*>(new TypedRaster<uint8_t>(dataType, res_x, res_y));
            break;
        case GDT_UInt16:
            ptr = static_cast<Raster*>(new TypedRaster<uint16_t>(dataType, res_x, res_y));
            break;
        case GDT_Int16:
            ptr = static_cast<Raster*>(new TypedRaster<int16_t>(dataType, res_x, res_y));
            break;
        case GDT_UInt32:
            ptr = static_cast<Raster*>(new TypedRaster<uint32_t>(dataType, res_x, res_y));
            break;
        case GDT_Int32:
            ptr = static_cast<Raster*>(new TypedRaster<int32_t>(dataType, res_x, res_y));
            break;
        case GDT_Float32:
            ptr = static_cast<Raster*>(new TypedRaster<float>(dataType, res_x, res_y));
            break;
        case GDT_Float64:
            ptr = static_cast<Raster*>(new TypedRaster<double>(dataType, res_x, res_y));
            break;

        default:
            throw std::runtime_error("Unsupported data type for raster creation.");
    }

    return std::unique_ptr<Raster>(ptr);
}

std::unique_ptr<Raster> Raster::createRaster(GDALDataType dataType, Resolution res) {
    return createRaster(dataType, res.res_x, res.res_y);
}

GDALDataType Raster::getDataType() const {
    return dataType;
}

Resolution Raster::getResolution() const {
    return Resolution(res_x, res_y);
}
