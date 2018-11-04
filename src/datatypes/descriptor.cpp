
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

SpatialReference Descriptor::calcCoordinatesOfTile() const {

    const SpatialReference &totalCoords = rasterInfo;
    const Resolution &totalRes = rasterInfo;
    const Resolution startOfTile = calcStartOfTile();

    if(totalRes.equalsResolution(tileResolution))
        return totalCoords;
    SpatialReference res;

    int x1 = startOfTile.res_x;
    int x2 = x1 + tileResolution.res_x;
    int y1 = startOfTile.res_y;
    int y2 = y1 + tileResolution.res_y;

    double perc_x1 = (float)x1 / totalRes.res_x;
    double perc_x2 = (float)x2 / totalRes.res_x;
    double perc_y1 = (float)y1 / totalRes.res_y;
    double perc_y2 = (float)y2 / totalRes.res_y;

    double coord_length_x = totalCoords.x2 - totalCoords.x1;
    double coord_length_y = totalCoords.y2 - totalCoords.y1;

    res.x1 = totalCoords.x1 + coord_length_x * perc_x1;
    res.x2 = totalCoords.x1 + coord_length_x * perc_x2;
    res.y1 = totalCoords.y1 + coord_length_y * perc_y1;
    res.y2 = totalCoords.y1 + coord_length_y * perc_y2;

    return res;
}

Resolution Descriptor::calcStartOfTile() const {
    //assumes that tile index is linear ordered, like follows:
    // 0 1 2 3
    // 4 5 6 7

    //calc how many x and y tiles the raster maximum has
    uint64_t max_tiles_x = rasterInfo.res_x / tileResolution.res_x;
    uint64_t max_tiles_y = rasterInfo.res_y / tileResolution.res_y;
    if(rasterInfo.res_x % tileResolution.res_x > 0)
        max_tiles_x += 1;
    if(rasterInfo.res_y % tileResolution.res_y > 0)
        max_tiles_y += 1;

    //calc start pixels from 1D-index
    Resolution start;
    start.res_x = (tileIndex % max_tiles_x) * tileResolution.res_x;
    start.res_y = (tileIndex / max_tiles_x) * tileResolution.res_y;
    return start;
}
