
#ifndef RASTER_TIME_SERIES_RASTER_CALC_UTIL_H
#define RASTER_TIME_SERIES_RASTER_CALC_UTIL_H

#include "datatypes/spatial_temporal_reference.h"

namespace rts {

    class RasterCalculations {
    public:
        /**
         *
         * @param rasterInfo
         * @param coord_x
         * @param coord_y
         * @return
         */
        static Resolution coordinateToWorldPixel(const SpatialTemporalReference &rasterInfo, double coord_x, double coord_y);

        /**
         *
         * @param rasterInfo
         * @return
         */
        static Resolution calcWorldResolution(const SpatialTemporalReference &rasterInfo);

        /**
         *
         * @param rasterInfo
         * @param pixelStart
         * @param pixelEnd
         * @return
         */
        static SpatialReference calcSpatialInfoFromPixel(const SpatialTemporalReference &rasterInfo,
                                                         Resolution pixelStart, Resolution pixelEnd);
    };

}

#endif //RASTER_TIME_SERIES_RASTER_CALC_UTIL_H
