
#include <iostream>
#include "datatypes/descriptor.h"
#include "descriptor.h"


using namespace rts;

Descriptor::Descriptor(std::function<UniqueRaster(const Descriptor&)> &&getter, SpatialTemporalReference &totalInfo, Resolution &tileResolution, Order order, uint32_t tileIndex, int nodata)
        : getter(std::move(getter)), rasterInfo(totalInfo), order(order), tileResolution(tileResolution), tileIndex(tileIndex), nodata(nodata) { }

Descriptor::Descriptor(std::function<UniqueRaster(const Descriptor &)> &&getter, SpatialTemporalReference &&totalInfo, Resolution &&tileResolution, Order order, uint32_t tileIndex, int nodata)
        : getter(std::move(getter)), rasterInfo(totalInfo), order(order), tileResolution(tileResolution), tileIndex(tileIndex), nodata(nodata) { }

Descriptor::Descriptor(std::function<UniqueRaster(const Descriptor &)> &&getter, SpatialTemporalReference &totalInfo, Resolution &&tileResolution, Order order, uint32_t tileIndex, int nodata)
        : getter(std::move(getter)), rasterInfo(totalInfo), order(order), tileResolution(tileResolution), tileIndex(tileIndex), nodata(nodata) { }

Descriptor::Descriptor(std::function<UniqueRaster(const Descriptor &)> &&getter, SpatialTemporalReference &&totalInfo, Resolution &tileResolution, Order order, uint32_t tileIndex, int nodata)
        : getter(std::move(getter)), rasterInfo(totalInfo), order(order), tileResolution(tileResolution), tileIndex(tileIndex), nodata(nodata) { }

uint64_t Descriptor::tilesOfRaster() const {
    //TODO: tiles should always start at (-180,-90) and by alligned in totals of their resolution. Therefore this is more complex, if raster coords are not the whole world. FIX!
    uint64_t x = rasterInfo.res_x / tileResolution.res_x;
    uint64_t y = rasterInfo.res_y / tileResolution.res_y;
    if(rasterInfo.res_x % tileResolution.res_x > 0)
        x += 1;
    if(rasterInfo.res_y % tileResolution.res_y > 0)
        y += 1;
    return x * y;
}

std::unique_ptr<Raster> Descriptor::getRaster() const {
    return getter(*this);
}

SpatialReference Descriptor::calculateCoordinatesOfTile() const {
    return SpatialReference();
}
