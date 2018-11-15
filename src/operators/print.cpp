
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

        SpatialReference tileCoords = desc.tileSpatialInfo;
        std::cout << desc.tileIndex << ": (" << desc.tileResolution.res_x << "," << desc.tileResolution.res_y << "), ("
                  << tileCoords.x1 << "," << tileCoords.x2 << "," << tileCoords.y1 << "," << tileCoords.y2
                  << "), (" << desc.rasterInfo.t1 << "," << desc.rasterInfo.t2 << ")" << std::endl;
        raster->print();
    }
}

bool Print::supportsOrder(Order o) {
    return o == Order::Temporal;
}
