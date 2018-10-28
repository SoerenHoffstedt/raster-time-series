
#include "print.h"

using namespace rts;

Print::Print(QueryRectangle qrect, Json::Value &params, const std::vector<GenericOperator*> in) : ConsumingOperator(qrect, params, in) {

}

OptionalDescriptor Print::next() {
    return std::nullopt;
}

void Print::consume() {
    for(auto &desc : *input_operators[0]){
        auto raster = desc.getRaster();

        std::cout << "(" << desc.tileInfo.res_x << "," << desc.tileInfo.res_y << "), (" << desc.tileInfo.x1 << "," << desc.tileInfo.x2 << "," << desc.tileInfo.y1 << "," <<
                  desc.tileInfo.y2 << "), (" << desc.tileInfo.t1 << "," << desc.tileInfo.t2 << ")" << std::endl;
        raster->print();
    }
}

bool Print::supportsOrder(Order o) {
    return o == Order::TemporalSpatial;
}
