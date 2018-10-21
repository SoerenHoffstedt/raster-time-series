
#include <iostream>
#include "datatypes/descriptor.h"
#include "descriptor.h"

using namespace rts;

Descriptor::Descriptor(QueryRectangle qrect_total, QueryRectangle qrect_tile) : tileInfo(qrect_tile), totalInfo(qrect_total) {

}

Descriptor::Descriptor(TemporalReference temp_ref_total, SpatialReference spat_ref_total, Resolution res_total,
                       TemporalReference temp_ref_tile, SpatialReference spat_ref_tile, Resolution res_tile, Order order)
                       : totalInfo(temp_ref_total, spat_ref_total, res_total, order),
                         tileInfo(temp_ref_tile, spat_ref_tile, res_tile, order) {

}
