
#include <stdexcept>
#include <iostream>
#include "datatypes/raster.h"
#include "raster.h"


using namespace rts;

Raster::Raster(int res_x, int res_y) : res_x(res_x), res_y(res_y), data_length(res_x * res_y) {
    data = new int[data_length];
}


Raster::Raster(Resolution res) : data_length(res.res_x * res.res_y) {
    res_x = res.res_x;
    res_y = res.res_y;
    data = new int[data_length];
}

Raster::Raster(const Raster &other) : data_length(other.data_length), res_x(other.res_x), res_y(other.res_y) {
    data = new int[data_length];
    std::copy(other.data, other.data + other.data_length, data);
}

Raster& Raster::operator=(const Raster &other) {
    delete[] data;
    data_length = other.data_length;
    res_x = other.res_x;
    res_y = other.res_y;
    data = new int[data_length];
    return *this;
}

//TODO: rule of five: implement move con., move assign.

Raster::~Raster()
{
    if(data != nullptr){
        delete[] data;
        data = nullptr;
    }
}

int* Raster::getDataPointer()
{
    return data;
}

int Raster::getCell(int x, int y) const {
    if(data == nullptr)
        throw std::runtime_error("Data of raster is not allocated");
    return data[x + y * res_x];
}

void Raster::setCell(int x, int y, int value){
    if(data == nullptr)
        throw std::runtime_error("Data of raster is not allocated");
    data[x + y * res_x] = value;
}

void Raster::print() const {

    for(int y = 0; y < res_y; y++){
        for(int x = 0; x < res_x; x++){
            int val = data[x + y * res_x];
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

int Raster::getDataLength() const {
    return data_length;
}
