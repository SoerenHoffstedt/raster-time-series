

#ifndef RASTER_TIME_SERIES_ORDER_H
#define RASTER_TIME_SERIES_ORDER_H

#include <stdint.h>

namespace rts {

    /**
    * The order of an operator returning the descriptors. First named dimension is the highest order, second the lowest.
    * Spatial order is fixed for now, only iterating the tiles of the raster line by line (advance all tiles in X
    * dimension, then advance Y by one, etc.
    * @Spatial returns one tile for each time, then advances to the next tile.
    * @Temporal returns all tiles from a raster, then advances in time to return the tiles of the next raster.
    */
    enum class Order {
        Spatial = 1,
        Temporal = 2
    };

}


#endif //RASTER_TIME_SERIES_ORDER_H
