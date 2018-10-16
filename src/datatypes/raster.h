
#ifndef RASTER_TIME_SERIES_RASTER_H
#define RASTER_TIME_SERIES_RASTER_H

#include <memory>
#include "datatypes/spatial_temporal_reference.h"

namespace rts {

    class Raster {
    public:
        Raster(int res_x, int res_y);
        Raster(Resolution res);
        ~Raster();
        int* getDataPointer();
        int getCell(int x, int y) const;
        void setCell(int x, int y, int value);
        void print() const;
    private:
        int* data;
        int res_x;
        int res_y;
        const int data_lenght;
    };
    using UniqueRaster = std::unique_ptr<Raster>;
}

#endif //RASTER_TIME_SERIES_RASTER_H
