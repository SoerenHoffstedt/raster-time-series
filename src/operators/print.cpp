
#include "print.h"

using namespace rts;

Print::Print(Json::Value &params, const std::vector<GenericOperator*> in) : GenericOperator(params, in) {

}

UniqueDescriptor Print::next() {
    for(auto &desc : *input_operators[0]){
        auto raster = desc.getRaster();
        raster->print();
    }
    return nullptr;
}
