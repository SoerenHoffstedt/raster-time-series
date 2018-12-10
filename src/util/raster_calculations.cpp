
#include "raster_calculations.h"
#include <cmath>

using namespace rts;

Resolution RasterCalculations::coordinateToWorldPixel(const SpatialTemporalReference &rasterInfo,
                                                      double coord_x,
                                                      double coord_y) {
    auto world_res = calcWorldResolution(rasterInfo);
    auto proj_extent = rasterInfo.projection.getExtent();

    double total_x = proj_extent.x2 - proj_extent.x1;
    double total_y = proj_extent.y2 - proj_extent.y1;

    double factor_x = (coord_x - proj_extent.x1) / total_x;
    double factor_y = (coord_y - proj_extent.y1) / total_y;

    Resolution result;
    result.res_x = (uint32_t)round(world_res.res_x * factor_x);
    result.res_y = (uint32_t)round(world_res.res_y * factor_y);
    return result;
}

Resolution RasterCalculations::calcWorldResolution(const SpatialTemporalReference&rasterInfo) {
    auto proj_extent = rasterInfo.projection.getExtent();
    double total_x = proj_extent.x2 - proj_extent.x1;
    double total_y = proj_extent.y2 - proj_extent.y1;

    double stretch_x = total_x / (rasterInfo.x2 - rasterInfo.x1);
    double stretch_y = total_y / (rasterInfo.y2 - rasterInfo.y1);

    Resolution world;
    world.res_x = (uint32_t)round(rasterInfo.res_x * stretch_x); //ceil or round? ceil to def. get all the data?
    world.res_y = (uint32_t)round(rasterInfo.res_y * stretch_y);
    return world;
}

SpatialReference RasterCalculations::calcSpatialInfoFromPixel(const SpatialTemporalReference &rasterInfo,
                                                              Resolution pixelStart,
                                                              Resolution pixelEnd) {
    SpatialReference extent = rasterInfo.projection.getExtent();
    Resolution worldRes = calcWorldResolution(rasterInfo);
    SpatialReference result;

    double total_x = extent.x2 - extent.x1;
    double total_y = extent.y2 - extent.y1;

    result.x1 = ((double)pixelStart.res_x / worldRes.res_x) * total_x + extent.x1;
    result.y1 = ((double)pixelStart.res_y / worldRes.res_y) * total_y + extent.y1;

    result.x2 = ((double)pixelEnd.res_x / worldRes.res_x)   * total_x + extent.x1;
    result.y2 = ((double)pixelEnd.res_y / worldRes.res_y)   * total_y + extent.y1;

    return result;
}
