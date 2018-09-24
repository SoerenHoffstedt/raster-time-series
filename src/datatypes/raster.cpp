
#include <stdexcept>
#include <iostream>
#include "datatypes/raster.h"
#include "raster.h"


using namespace rts;

Raster::Raster(int res_x, int res_y) : res_x(res_x), res_y(res_y), data_lenght(res_x * res_y) {
    data = new int[res_x * res_y];
}

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

int Raster::getCell(int x, int y) const{
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
            std::cout << data[x + y * res_x] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}
