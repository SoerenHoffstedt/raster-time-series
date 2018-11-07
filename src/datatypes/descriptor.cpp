
#include <iostream>
#include "datatypes/descriptor.h"
#include "descriptor.h"
#include <cmath>


using namespace rts;

Descriptor::Descriptor(std::function<UniqueRaster(const Descriptor&)> &&getter, const SpatialTemporalReference &totalInfo, const Resolution &tileResolution, Order order, uint32_t tileIndex, int nodata)
        : getter(std::move(getter)), rasterInfo(totalInfo), order(order), tileResolution(tileResolution), tileIndex(tileIndex), nodata(nodata), _isOnlyNodata(false) { }

uint64_t Descriptor::tilesOfRaster() const {
    //TODO: tiles should always start at (-180,-90) and by alligned in totals of their resolution.
    // Therefore this is more complex, if raster coords are not the whole world. FIX!

    Resolution world_res = calcWorldResolution();

    uint64_t x = rasterInfo.res_x / tileResolution.res_x;
    uint64_t y = rasterInfo.res_y / tileResolution.res_y;
    if(rasterInfo.res_x % tileResolution.res_x > 0)
        x += 1;
    if(rasterInfo.res_y % tileResolution.res_y > 0)
        y += 1;
    return x * y;
}

Resolution Descriptor::calcWorldResolution() const {

    double stretch_x = 360.0 / (rasterInfo.x2 - rasterInfo.x1);
    double stretch_y = 180.0 / (rasterInfo.y2 - rasterInfo.y1);

    Resolution world;
    world.res_x = (uint32_t)ceil(rasterInfo.res_x * stretch_x); //ceil or round? ceil to def. get all the data?
    world.res_y = (uint32_t)ceil(rasterInfo.res_y * stretch_y);

    return world;
}

std::unique_ptr<Raster> Descriptor::getRaster() const {
    return getter(*this);
}

SpatialReference Descriptor::calcCoordinatesOfTile() const {

    const SpatialReference &totalCoords = rasterInfo;
    const Resolution &totalRes = rasterInfo;
    const Resolution startPixelOfTile = calcStartPixelOfTile();

    if(totalRes.equalsResolution(tileResolution))
        return totalCoords;
    SpatialReference res;

    int x1 = startPixelOfTile.res_x;
    int x2 = x1 + tileResolution.res_x;
    int y1 = startPixelOfTile.res_y;
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

    //TODO: if tile size is bigger than query resolution: x2/y2 will be bigger than 180/90. how to handle this?
    // cap the values to the world coords? and simply give more data in the tile?
    if(res.x2 > 180.0)
        res.x2 = 180.0;
    if(res.y2 > 90.0)
        res.y2 = 90.0;

    return res;
}

Resolution Descriptor::calcStartPixelOfTile() const {
    //assumes that tile index is linear ordered, like follows:
    // 0 1 2 3
    // 4 5 6 7

    //TODO: this wont work with alligned tiles. would need to calculate the whole world resolution first, because
    // tiles dont start at (x1,y1) of the qrect but at (-180,-90), so it could be more tiles than this.
    // first solve it in tilesOfRaster, than adapt it here, better make it reusable for both calculations.

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

bool Descriptor::isOnlyNodata() const {
    return _isOnlyNodata;
}

std::optional<Descriptor> Descriptor::createNodataDescriptor(SpatialTemporalReference &totalInfo, Resolution &tileResolution, Order order, uint32_t tileIndex, int nodata) {
    auto getter = [tileResolution = tileResolution](const Descriptor &self) -> UniqueRaster {
        UniqueRaster raster = std::make_unique<Raster>(tileResolution);
        for (int x = 0; x < tileResolution.res_x; ++x) {
            for (int y = 0; y < tileResolution.res_y; ++y) {
                raster->setCell(x, y, self.nodata);
            }
        }
        return raster;
    };
    auto ret = std::make_optional<Descriptor>(std::move(getter), totalInfo, tileResolution, order, tileIndex, nodata);
    ret->_isOnlyNodata = true;
    return ret;
}
