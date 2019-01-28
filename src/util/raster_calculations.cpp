
#include "raster_calculations.h"
#include <cmath>

using namespace rts;

Resolution RasterCalculations::coordinateToPixel(const Scale& scale, const Origin &origin, double coordX, double coordY){
    Resolution res;
    res.resX = static_cast<uint32_t>((coordX - origin.x) / scale.x);
    res.resY = static_cast<uint32_t>((coordY - origin.y) / scale.y);
    return res;
}

std::pair<double,double> RasterCalculations::pixelToCoordinate(const Scale& scale, const Origin &origin, Resolution pixelCoord) {
    return pixelToCoordinate(scale, origin, pixelCoord.resX, pixelCoord.resY);
}

std::pair<double,double> RasterCalculations::pixelToCoordinate(const Scale& scale, const Origin &origin, uint32_t pixelX, uint32_t pixelY) {
    std::pair<double, double> res;
    res.first  = origin.x + scale.x * pixelX;
    res.second = origin.y + scale.y * pixelY;
    return res;
}

Resolution RasterCalculations::coordinateToPixel(const SpatialTemporalReference &rasterInfo,
                                                 double coord_x,
                                                 double coord_y)
{
    return coordinateToPixel(rasterInfo, rasterInfo, coord_x, coord_y);
}


Resolution RasterCalculations::coordinateToPixel(const Resolution &rasterSize,
                                                 const SpatialReference &rasterExtent,
                                                 double coord_x,
                                                 double coord_y)
{

    double total_x = rasterExtent.x2 - rasterExtent.x1;
    double total_y = rasterExtent.y2 - rasterExtent.y1;

    double factor_x = (coord_x - rasterExtent.x1) / total_x;
    double factor_y = (coord_y - rasterExtent.y1) / total_y;

    Resolution result;
    result.resX = (uint32_t)round(rasterSize.resX * factor_x);
    result.resY = (uint32_t)round(rasterSize.resY * factor_y);
    return result;
}

SpatialReference RasterCalculations::pixelToSpatialRectangle(const SpatialTemporalReference &rasterInfo,
                                                             Resolution pixelStart,
                                                             Resolution pixelEnd)
{
    return pixelToSpatialRectangle(rasterInfo, rasterInfo, pixelStart, pixelEnd);
}



SpatialReference RasterCalculations::pixelToSpatialRectangle(const Resolution &rasterSize,
                                                             const SpatialReference &rasterExtent,
                                                             Resolution pixelStart,
                                                             Resolution pixelEnd) {
    SpatialReference result;

    double total_x = rasterExtent.x2 - rasterExtent.x1;
    double total_y = rasterExtent.y2 - rasterExtent.y1;

    result.x1 = ((double)pixelStart.resX / rasterSize.resX) * total_x + rasterExtent.x1;
    result.y1 = ((double)pixelStart.resY / rasterSize.resY) * total_y + rasterExtent.y1;

    result.x2 = ((double)pixelEnd.resX / rasterSize.resX)   * total_x + rasterExtent.x1;
    result.y2 = ((double)pixelEnd.resY / rasterSize.resY)   * total_y + rasterExtent.y1;

    result.projection = rasterExtent.projection;
    return result;
}

SpatialReference RasterCalculations::pixelToSpatialRectangle(const Scale &scale, const Origin &origin,
                                                             Resolution pixelStart, Resolution pixelEnd)
{
    SpatialReference result;
    result.x1 = origin.x + pixelStart.resX * scale.x;
    result.y1 = origin.y + pixelStart.resY * scale.y;
    result.x2 = origin.x + pixelEnd.resX   * scale.x;
    result.y2 = origin.y + pixelEnd.resY   * scale.y;
    return result;
}