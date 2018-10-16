
#include <iostream>
#include "datatypes/descriptor.h"
#include "descriptor.h"

using namespace rts;

Descriptor::Descriptor(double time_start, double time_end, double x1, double x2, double y1, double y2,
                       uint32_t res_x, uint32_t res_y, Order order)
       : st_ref(time_start, time_end, x1, x2, y1, y2, res_x, res_y, order) {

}

Descriptor::Descriptor(TemporalReference temp_ref, SpatialReference spat_ref, Resolution res, Order order)
        : st_ref(temp_ref, spat_ref, res, order) {

}

Descriptor::Descriptor(QueryRectangle st_ref) : st_ref(st_ref) {

}
