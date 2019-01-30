
#include "datatypes/spatial_temporal_reference.h"
#include "spatial_temporal_reference.h"

using namespace rts;

// Scale (& Origin) deftinitions:
Scale::Scale(double x, double y) : x(x), y(y)
{

}

Scale::Scale() : x(1), y(1)
{

}


// Projection definitions:

Projection::Projection() : code(4326), authority("EPSG") { }

Projection::Projection(std::string authority, uint32_t code) : authority(std::move(authority)), code(code) { }

SpatialReference Projection::getExtent() const {
    //copied from mapping
    if (authority == "EPSG" && code == 3857)
        return SpatialReference(-20037508.34, 20037508.34, -20037508.34, 20037508.34);
    else if (authority == "EPSG" && code == 4326)
        return SpatialReference(-180, 180, -90, 90);
    else if (authority == "SR-ORG" && code == 81 )
        return SpatialReference(-5568748.276, 5568748.276, -5568748.276, 5568748.276);
    else
        return SpatialReference(-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
}

Projection::Projection(const Json::Value &spatial_def) : Projection(spatial_def["projection"].asString()) {

}

Projection::Projection(const std::string &proj) {
    auto separation = proj.find(':');
    authority = proj.substr(0, separation);
    code = std::stoi(proj.substr(separation + 1));
}

// Temporal Reference definitions:

TemporalReference::TemporalReference(double t1, double t2) : t1(t1), t2(t2) { }

TemporalReference::TemporalReference(const Json::Value &temp) {
    t1 = temp["start"].asDouble();
    t2 = temp["end"].asDouble();
}
TemporalReference::TemporalReference() : t1(0.0), t2(0.0) { }

TemporalReference TemporalReference::getOverlapTemporal(const TemporalReference &o) const {
    return TemporalReference(t1 > o.t1 ? t1 : o.t1, t2 < o.t2 ? t2 : o.t2);
}
bool TemporalReference::overlapsWithTemporal(const TemporalReference &other) const {
    return t1 <= other.t2 && t2 >= other.t1;
}

double TemporalReference::beginningOfTime() const {
    // copies from mapping, atm this only has unix as timetype.
    // A test in unittests/temporal/timeparser.cpp verifies that the constant matches the given date
    // ISO 8601: 0001-01-01T00:00:00
    return -62135596800;
}

double TemporalReference::endOfTime() const {
    // copies from mapping, atm this only has unix as timetype.
    // A test in unittests/temporal/timeparser.cpp verifies that the constant matches the given date
    // ISO 8601: 9999-12-31T23:59:59
    return 253402300799;
}

bool TemporalReference::containsTemporal(double t) const {
    return t >= t1 && t <= t2;
}

// Spatial Reference definitions:

SpatialReference::SpatialReference(double x1, double x2, double y1, double y2) : x1(x1), x2(x2), y1(y1), y2(y2), projection() { }

SpatialReference::SpatialReference() : projection() {
    auto extent = projection.getExtent();
    x1 = extent.x1;
    x2 = extent.x2;
    y1 = extent.y1;
    y2 = extent.y2;
}

SpatialReference::SpatialReference(const Json::Value &spat) : projection(spat["projection"].asString()) {
    x1 = spat["x1"].asDouble();
    x2 = spat["x2"].asDouble();
    y1 = spat["y1"].asDouble();
    y2 = spat["y2"].asDouble();
}

bool SpatialReference::equalsSpatial(const SpatialReference &other) const {
    //TODO: add delta for float inaccuracy?
    return x1 == other.x1 && x2 == other.x2 && y1 == other.y1 && y2 == other.y2;
}

bool SpatialReference::overlapsWithSpatial(const SpatialReference &other) const {
    return x1 <= other.x2 && x2 >= other.x1 && y1 <= other.y2 && y2 >= other.y1;
}

SpatialReference SpatialReference::getOverlapSpatial(const SpatialReference &other) const {
    if(!overlapsWithSpatial(other)){
        return SpatialReference(0,0,0,0);
    }
    double xx1 = x1 > other.x1 ? x1 : other.x1;
    double xx2 = x2 < other.x2 ? x2 : other.x2;
    double yy1 = y1 > other.y1 ? y1 : other.y1;
    double yy2 = y2 < other.y2 ? y2 : other.y2;
    return SpatialReference(xx1, xx2, yy1, yy2);
}

bool SpatialReference::containsSpatial(double x, double y) const {
    return x >= x1 && x <= x2 && y >= y1 && y <= y2;
}

// Resolution definitions:

Resolution::Resolution(uint32_t x, uint32_t y) : resX(x), resY(y) { }

Resolution::Resolution() : resX(0), resY(0) { }

Resolution::Resolution(const Json::Value &res){
    resX = res["x"].asUInt();
    resY = res["y"].asUInt();
}

bool Resolution::equalsResolution(const Resolution &other) const {
    return resX == other.resX && resY == other.resY;
}

Resolution Resolution::operator+(const Resolution &other) const {
    return Resolution(this->resX + other.resX, this->resY + other.resY);
}

Resolution Resolution::operator-(const Resolution &other) const {
    return Resolution(this->resX - other.resX, this->resY - other.resY);
}

// Spatial Temporal Reference definitions:

SpatialTemporalReference::SpatialTemporalReference(double t1, double t2, double x1, double x2, double y1, double y2, uint32_t res_x, uint32_t res_y)
        : TemporalReference(t1, t2), SpatialReference(x1, x2, y1, y2), Resolution(res_x, res_y), scale((x2 - x1) / res_x, (y2 - y1) / res_y)
{

}

SpatialTemporalReference::SpatialTemporalReference(const TemporalReference &temp_ref, const SpatialReference &spat_ref, const Resolution &res)
        : TemporalReference(temp_ref), SpatialReference(spat_ref), Resolution(res),
          scale((x2 - x1) / resX, (y2 - y1) / resY)
{

}

SpatialTemporalReference::SpatialTemporalReference(const Json::Value &qrect)
        : TemporalReference(qrect["temporal_reference"]), SpatialReference(qrect["spatial_reference"]),
          Resolution(qrect["resolution"]), scale((x2 - x1) / resX, (y2 - y1) / resY)
{

}

// Query Rectangle defintions:

QueryRectangle::QueryRectangle(double t1, double t2, double x1, double x2, double y1, double y2, uint32_t res_x, uint32_t res_y, Order order, Resolution tileRes)
        : SpatialTemporalReference(t1,t2, x1, x2, y1, y2, res_x, res_y), order(order), tileRes(tileRes) { }

QueryRectangle::QueryRectangle(const TemporalReference &temp_ref, const SpatialReference &spat_ref, const Resolution &res, Order order, Resolution tileRes)
        : SpatialTemporalReference(temp_ref, spat_ref, res), order(order), tileRes(tileRes) { }

QueryRectangle::QueryRectangle(const Json::Value &qrect)
        : SpatialTemporalReference(qrect)
{
    std::string order_str = qrect["order"].asString();
    if(order_str == "Spatial")
        order = Order::Spatial;
    else if(order_str == "Temporal")
        order = Order::Temporal;
    else
        throw std::runtime_error("Invalid value for order enum: " + order_str);
    Json::Value tileResJson = qrect["tileRes"];
    tileRes.resX = tileResJson["x"].asUInt();
    tileRes.resY = tileResJson["y"].asUInt();
}
