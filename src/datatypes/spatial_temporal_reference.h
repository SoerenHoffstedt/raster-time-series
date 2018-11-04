
#ifndef RASTER_TIME_SERIES_ST_REF_H
#define RASTER_TIME_SERIES_ST_REF_H

#include <cstdint>
#include <iostream>
#include "datatypes/order.h"
#include <json/json.h>

namespace rts {

    class TemporalReference {
    public:
        TemporalReference(double t1, double t2) : t1(t1), t2(t2) { }
        TemporalReference(const Json::Value &temp) {
            t1 = temp["start"].asDouble();
            t2 = temp["end"].asDouble();
        }
        TemporalReference() : t1(0.0), t2(0.0) { }
        TemporalReference getOverlapTemporal(const TemporalReference &o){
            return TemporalReference(t1 > o.t1 ? t1 : o.t1, t2 < o.t2 ? t2 : o.t2);
        }
        bool overlapsWithTemporal(const TemporalReference &other){
            return t1 <= other.t2 && t2 >= other.t1;
        }
        double t1;
        double t2;
    };

    class SpatialReference {
    public:
        SpatialReference(double x1, double x2, double y1, double y2) : x1(x1), x2(x2), y1(y1), y2(y2) { }
        SpatialReference() : x1(-180), x2(180), y1(-90), y2(90) { }
        SpatialReference(const Json::Value &spat){
            x1 = spat["x1"].asDouble();
            x2 = spat["x2"].asDouble();
            y1 = spat["y1"].asDouble();
            y2 = spat["y2"].asDouble();
        }
        bool equalsSpatial(const SpatialReference &other){
            //TODO: add delta for float inaccuracy?
            return x1 == other.x1 && x2 == other.x2 && y1 == other.y1 && y2 == other.y2;
        }
        SpatialReference getOverlapSpatial(const SpatialReference &other){
            if(!overlapsWithSpatial(other)){
                return SpatialReference(0,0,0,0);
            }
            double xx1 = x1 > other.x1 ? x1 : other.x1;
            double xx2 = x2 < other.x2 ? x2 : other.x2;
            double yy1 = y1 > other.y1 ? y1 : other.y1;
            double yy2 = y2 < other.y2 ? y2 : other.y2;
            return SpatialReference(xx1, xx2, yy1, yy2);
        }
        bool overlapsWithSpatial(const SpatialReference &other){
            return x1 <= other.x2 && x2 >= other.x1 && y1 <= other.y2 && y2 >= other.y1;
        }
        double x1;
        double x2;
        double y1;
        double y2;
    };

    class Resolution {
    public:
        Resolution(uint32_t x, uint32_t y) : res_x(x), res_y(y) { }
        Resolution() : res_x(0), res_y(0) { }
        Resolution(const Json::Value &res){
            res_x = res["x"].asUInt();
            res_y = res["y"].asUInt();
        }
        bool equalsResolution(const Resolution &other) const {
            return res_x == other.res_x && res_y == other.res_y;
        }
        uint32_t res_x;
        uint32_t res_y;
    };

    class SpatialTemporalReference : public TemporalReference, public SpatialReference, public Resolution {
    public:
        SpatialTemporalReference(double t1, double t2, double x1, double x2, double y1, double y2, uint32_t res_x, uint32_t res_y)
                : TemporalReference(t1, t2), SpatialReference(x1, x2, y1, y2), Resolution(res_x, res_y){ }

        SpatialTemporalReference(const TemporalReference &temp_ref, const SpatialReference &spat_ref, const Resolution &res)
                : TemporalReference(temp_ref), SpatialReference(spat_ref), Resolution(res) { }

        explicit SpatialTemporalReference(const Json::Value &qrect)
                : TemporalReference(qrect["temporal_reference"]),
                  SpatialReference(qrect["spatial_reference"]),
                  Resolution(qrect["resolution"]) { }
        SpatialTemporalReference(const SpatialTemporalReference &other) = default;
    };

    class QueryRectangle : public SpatialTemporalReference {
    public:
        QueryRectangle(double t1, double t2, double x1, double x2, double y1, double y2, uint32_t res_x, uint32_t res_y, Order order)
            : SpatialTemporalReference(t1,t2, x1, x2, y1, y2, res_x, res_y), order(order) { }

        QueryRectangle(const TemporalReference &temp_ref, const SpatialReference &spat_ref, const Resolution &res, Order order)
            : SpatialTemporalReference(temp_ref, spat_ref, res), order(order) { }

        explicit QueryRectangle(const Json::Value &qrect)
                : SpatialTemporalReference(qrect)
        {
            std::string order_str = qrect["order"].asString();
            if(order_str == "SpatialTemporal")
                order = Order::SpatialTemporal;
            else if(order_str == "TemporalSpatial")
                order = Order::TemporalSpatial;
            else
                throw std::runtime_error("Invalid value for order enum: " + order_str);
        }
        QueryRectangle(const QueryRectangle &other) = default;
        Order order;
    };

}

#endif //RASTER_TIME_SERIES_ST_REF_H
