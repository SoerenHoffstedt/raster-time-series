
#ifndef RASTER_TIME_SERIES_RASTER_H
#define RASTER_TIME_SERIES_RASTER_H

namespace rts {

    class Raster {
    public:
        Raster(int res_x, int res_y);
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
}

#endif //RASTER_TIME_SERIES_RASTER_H
