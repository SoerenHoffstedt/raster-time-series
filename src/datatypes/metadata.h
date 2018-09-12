
#ifndef RASTER_TIME_SERIES_METADATA_H
#define RASTER_TIME_SERIES_METADATA_H

namespace rts {

    class MetaData {
    public:
        MetaData() {

        }

        MetaData(double x1, double x2, double y1, double y2) : x1(x1), x2(x2), y1(y1), y2(y2){

        }

        double x1;
        double x2;
        double y1;
        double y2;
    };

}

#endif //RASTER_TIME_SERIES_METADATA_H
