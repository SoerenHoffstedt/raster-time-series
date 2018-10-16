
#include "print.h"

using namespace rts;

Print::Print(QueryRectangle qrect, Json::Value &params, const std::vector<GenericOperator*> in) : ConsumingOperator(qrect, params, in) {

}

UniqueDescriptor Print::next() {
    return nullptr;
}

void Print::consume() {
    for(auto &desc : *input_operators[0]){
        auto raster = desc.getRaster();
        std::cout << "(" << desc.st_ref.res_x << "," << desc.st_ref.res_y << "), (" << desc.st_ref.x1 << "," << desc.st_ref.x2 << "," << desc.st_ref.y1 << "," <<
                  desc.st_ref.y2 << "), (" << desc.st_ref.t1 << "," << desc.st_ref.t2 << ")" << std::endl;
        raster->print();
    }
}

bool Print::supportsOrder(Order o) {
    return o == Order::TemporalSpatial;
}
