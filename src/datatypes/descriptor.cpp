
#include <iostream>
#include "datatypes/descriptor.h"
#include "descriptor.h"


using namespace rts;

Descriptor::Descriptor(std::function<UniqueRaster(const Descriptor&)> &&getter, QueryRectangle &qrect_total, QueryRectangle &qrect_tile)
        : getter(std::move(getter)), totalInfo(qrect_total), tileInfo(qrect_tile) { }

Descriptor::Descriptor(std::function<UniqueRaster(const Descriptor &)> &&getter, QueryRectangle &&qrect_total, QueryRectangle &&qrect_tile)
        : getter(std::move(getter)), totalInfo(qrect_total), tileInfo(qrect_tile) { }

Descriptor::Descriptor(std::function<UniqueRaster(const Descriptor &)> &&getter, QueryRectangle &qrect_total, QueryRectangle &&qrect_tile)
        : getter(std::move(getter)), totalInfo(qrect_total), tileInfo(qrect_tile) { }

Descriptor::Descriptor(std::function<UniqueRaster(const Descriptor &)> &&getter, QueryRectangle &&qrect_total, QueryRectangle &qrect_tile)
        : getter(std::move(getter)), totalInfo(qrect_total), tileInfo(qrect_tile) { }

uint64_t Descriptor::tilesOfRaster() const {
    uint64_t x = totalInfo.res_x / tileInfo.res_x;
    uint64_t y = totalInfo.res_y / tileInfo.res_y;
    if(totalInfo.res_x % tileInfo.res_x > 0)
        x += 1;
    if(totalInfo.res_y % tileInfo.res_y > 0)
        y += 1;
    return x * y;
}

std::unique_ptr<Raster> Descriptor::getRaster() const {
    return getter(*this);
}
