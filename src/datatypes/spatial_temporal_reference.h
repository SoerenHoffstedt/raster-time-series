
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
            res_x = res["x"].asInt();
            res_y = res["y"].asInt();
        }
        uint32_t res_x;
        uint32_t res_y;
    };

    class QueryRectangle : public TemporalReference, public SpatialReference, public Resolution {
    public:
        QueryRectangle(double t1, double t2, double x1, double x2, double y1, double y2, uint32_t res_x, uint32_t res_y, Order order)
            : TemporalReference(t1, t2), SpatialReference(x1, x2, y1, y2), Resolution(res_x, res_y), order(order) { }

        QueryRectangle(const TemporalReference &temp_ref, const SpatialReference &spat_ref, const Resolution &res, Order order)
            : TemporalReference(temp_ref), SpatialReference(spat_ref), Resolution(res), order(order) { }

        QueryRectangle(const Json::Value &qrect)
                : TemporalReference(qrect["temporal_reference"]),
                  SpatialReference(qrect["spatial_reference"]),
                  Resolution(qrect["resolution"])
        {
            std::string order_str = qrect["order"].asString();
            if(order_str == "SpatialTemporal")
                order = Order::SpatialTemporal;
            else if(order_str == "TemporalSpatial")
                order = Order::TemporalSpatial;
            else
                throw std::runtime_error("Invalid value for order enum: " + order_str);
        }
        Order order;
    };

}

#endif //RASTER_TIME_SERIES_ST_REF_H
