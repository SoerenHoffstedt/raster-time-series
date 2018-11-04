
#include "print.h"

using namespace rts;

Print::Print(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in) : ConsumingOperator(qrect, params, std::move(in)) {

}

OptionalDescriptor Print::next() {
    return std::nullopt;
}

void Print::consume() {
    for(auto &desc : *input_operators[0]){
        auto raster = desc.getRaster();

        //TODO: print tile coords, not whole raster coords, but function is not implemented yet.
        SpatialReference tileCoords = desc.calculateCoordinatesOfTile();
        std::cout << desc.tileIndex << ": (" << desc.tileResolution.res_x << "," << desc.tileResolution.res_y << "), (" << desc.rasterInfo.x1 << "," << desc.rasterInfo.x2 << "," << desc.rasterInfo.y1 << "," <<
                  desc.rasterInfo.y2 << "), (" << desc.rasterInfo.t1 << "," << desc.rasterInfo.t2 << ")" << std::endl;
        raster->print();
    }
}

bool Print::supportsOrder(Order o) {
    return o == Order::TemporalSpatial;
}
