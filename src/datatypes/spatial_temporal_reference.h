
#ifndef RASTER_TIME_SERIES_ST_REF_H
#define RASTER_TIME_SERIES_ST_REF_H

#include <cstdint>
#include <iostream>
#include "datatypes/order.h"
#include <json/json.h>

namespace rts {

    class SpatialReference;

    class Projection {
    public:
        //TODO: not handling projection right at the moment, for now everything is EPSG:4326. fix that.
        Projection();
        Projection(std::string authority, uint32_t code);
        std::string authority;
        uint32_t code;

        /**
         * @return The coordinates maximum extent of this projection.
         */
        SpatialReference getExtent() const;
    };

    class TemporalReference {
    public:
        TemporalReference();
        TemporalReference(double t1, double t2);
        explicit TemporalReference(const Json::Value &temp);
        TemporalReference getOverlapTemporal(const TemporalReference &o) const;
        bool overlapsWithTemporal(const TemporalReference &other) const;

        double beginningOfTime() const;
        double endOfTime() const;

        double t1;
        double t2;
    };

    class SpatialReference {
    public:
        SpatialReference();
        SpatialReference(double x1, double x2, double y1, double y2);
        explicit SpatialReference(const Json::Value &spat);
        bool overlapsWithSpatial(const SpatialReference &other) const;
        bool equalsSpatial(const SpatialReference &other)  const;
        SpatialReference getOverlapSpatial(const SpatialReference &other) const;

        double x1;
        double x2;
        double y1;
        double y2;
        Projection projection;
    };

    class Resolution {
    public:
        Resolution(uint32_t x, uint32_t y);
        Resolution();
        explicit Resolution(const Json::Value &res);
        bool equalsResolution(const Resolution &other) const;
        Resolution operator+(const Resolution &other) const;
        uint32_t res_x;
        uint32_t res_y;
    };

    class SpatialTemporalReference : public TemporalReference, public SpatialReference, public Resolution {
    public:
        SpatialTemporalReference(double t1, double t2, double x1, double x2, double y1, double y2, uint32_t res_x, uint32_t res_y);
        SpatialTemporalReference(const TemporalReference &temp_ref, const SpatialReference &spat_ref, const Resolution &res);
        explicit SpatialTemporalReference(const Json::Value &qrect);
        SpatialTemporalReference(const SpatialTemporalReference &other) = default;
    };

    class QueryRectangle : public SpatialTemporalReference {
    public:
        QueryRectangle(double t1, double t2, double x1, double x2, double y1, double y2, uint32_t res_x, uint32_t res_y, Order order);
        QueryRectangle(const TemporalReference &temp_ref, const SpatialReference &spat_ref, const Resolution &res, Order order);
        explicit QueryRectangle(const Json::Value &qrect);
        QueryRectangle(const QueryRectangle &other) = default;
        Order order;
    };

}

#endif //RASTER_TIME_SERIES_ST_REF_H
