
#include "print.h"

using namespace rts;

Print::Print(Json::Value &params, const std::vector<GenericOperator*> in) : ConsumingOperator(params, in) {

}

UniqueDescriptor Print::next() {
    return nullptr;
}

void Print::consume() {
    for(auto &desc : *input_operators[0]){
        auto raster = desc.getRaster();
        raster->print();
    }
}
