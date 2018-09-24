
#ifndef RASTER_TIME_SERIES_ST_REF_H
#define RASTER_TIME_SERIES_ST_REF_H

#include <cstdint>

namespace rts {


    class TemporalReference {
    public:
        TemporalReference(double t1, double t2) : t1(t1), t2(t2) { }
        double t1;
        double t2;
    };

    class SpatialReference {
    public:
        SpatialReference(double x1, double x2, double y1, double y2) : x1(x1), x2(x2), y1(y1), y2(y2) { }
        double x1;
        double x2;
        double y1;
        double y2;
    };

    class Resolution {
    public:
        Resolution(uint32_t x, uint32_t y) : res_x(x), res_y(y) { }
        uint32_t res_x;
        uint32_t res_y;
    };

    class SpatialTemporalReference : public TemporalReference, public SpatialReference, public Resolution {
    public:
        SpatialTemporalReference(double t1, double t2, double x1, double x2, double y1, double y2, uint32_t res_x, uint32_t res_y)
            : TemporalReference(t1, t2), SpatialReference(x1, x2, y1, y2), Resolution(res_x, res_y) { }
        SpatialTemporalReference(const TemporalReference &temp_ref, const SpatialReference &spat_ref, const Resolution &res)
            : TemporalReference(temp_ref), SpatialReference(spat_ref), Resolution(res) { }
    };

}

#endif //RASTER_TIME_SERIES_ST_REF_H
