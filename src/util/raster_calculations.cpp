
#include "raster_calculations.h"
#include <cmath>

using namespace rts;

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
    result.res_x = (uint32_t)round(rasterSize.res_x * factor_x);
    result.res_y = (uint32_t)round(rasterSize.res_y * factor_y);
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

    result.x1 = ((double)pixelStart.res_x / rasterSize.res_x) * total_x + rasterExtent.x1;
    result.y1 = ((double)pixelStart.res_y / rasterSize.res_y) * total_y + rasterExtent.y1;

    result.x2 = ((double)pixelEnd.res_x / rasterSize.res_x)   * total_x + rasterExtent.x1;
    result.y2 = ((double)pixelEnd.res_y / rasterSize.res_y)   * total_y + rasterExtent.y1;

    result.projection = rasterExtent.projection;
    return result;
}
