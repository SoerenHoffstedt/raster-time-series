
#include <iostream>
#include "datatypes/descriptor.h"
#include "descriptor.h"

using namespace rts;

Descriptor::Descriptor(int id,
                       int res_x,
                       int res_y,
                       double time_start,
                       double time_duration,
                       std::vector<Descriptor*> inputs,
                       std::shared_ptr<GenericOperator> operator_ptr)
        : id(id),
          res_x(res_x),
          res_y(res_y),
          time_start(time_start),
          time_duration(time_duration),
          inputs(std::move(inputs)),
          raster_ptr(nullptr),
          operator_ptr(std::move(operator_ptr))
{
    min = 0;
    max = 256;
}

Raster* Descriptor::getRasterPtr(){
    if(raster_ptr == nullptr){
        raster_ptr = operator_ptr->executeOnRaster(this);
    }
    return raster_ptr;
}

bool Descriptor::isRasterLoaded() const {
    return raster_ptr != nullptr;
}

bool Descriptor::isValidBetween(double low, double high) const {
    double time_end = time_start + time_duration;
    return time_start >= low && time_start < high
            || time_end >= low && time_end < high
            || time_start <= low && time_end >= high;
}

Descriptor::~Descriptor() {
    if(raster_ptr != nullptr){
        delete raster_ptr;
        raster_ptr = nullptr;
    }
}
