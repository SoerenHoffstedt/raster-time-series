
#ifndef RASTER_TIME_SERIES_RASTER_H
#define RASTER_TIME_SERIES_RASTER_H

#include <memory>
#include "datatypes/spatial_temporal_reference.h"

namespace rts {

    class Raster {
    public:
        Raster(int res_x, int res_y);
        Raster(Resolution res);
        Raster(const Raster &other);
        Raster& operator=(const Raster &other);
        Raster(Raster &&other) noexcept;
        Raster& operator=(Raster &&other) noexcept;
        ~Raster();
        int* getDataPointer();
        int getCell(int x, int y) const;
        void setCell(int x, int y, int value);
        void print() const;
        int getDataLength() const;
    private:
        int* data;
        int res_x;
        int res_y;
        int data_length;
    };
    using UniqueRaster = std::unique_ptr<Raster>;
}

#endif //RASTER_TIME_SERIES_RASTER_H
