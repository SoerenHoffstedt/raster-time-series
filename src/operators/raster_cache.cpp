
#include "operators/raster_cache.h"


rts::RasterCache::RasterCache(const rts::OperatorTree *operator_tree, const rts::QueryRectangle &qrect,
                                    const Json::Value &params, rts::UniqueOperatorVector &&in)
                                    : GenericOperator(operator_tree, qrect, params, std::move(in)), lastTileIndex(0)
{
    checkInputCount(1);
}

void rts::RasterCache::initialize() {

}

bool rts::RasterCache::supportsOrder(rts::Order order) const {
    return order == Order::Temporal;
}

rts::OptionalDescriptor rts::RasterCache::nextDescriptor() {
    auto input = input_operators[0]->nextDescriptor();

    if(!input)
        return boost::none;

    if(input->tileIndex == 0){ //new raster
        cache.clear();
        if(cache.capacity() < input->rasterTileCount)
            cache.reserve(input->rasterTileCount);
        for(int i = 0; i < input->rasterTileCount; ++i){
            cache.emplace_back(nullptr);
        }
    }

    return createOutput(input);
}

rts::OptionalDescriptor rts::RasterCache::getDescriptor(int tileIndex) {
    auto input = input_operators[0]->getDescriptor(tileIndex);
    if(!input)
        return boost::none;
    return createOutput(input);
}

rts::OptionalDescriptor rts::RasterCache::createOutput(OptionalDescriptor &input) {
    lastTileIndex = input->tileIndex;

    DescriptorInfo info(input);

    auto getter = [input = std::move(input), &cache = cache](const Descriptor &self) -> UniqueRaster {
        if(cache[self.tileIndex] == nullptr){
            cache[self.tileIndex] = input->getRaster();
        }
        auto res = cache[self.tileIndex]->getResolution();
        return Raster::createRaster(self.dataType, res.resX, res.resY, cache[self.tileIndex]->getVoidDataPointer());
    };


    return rts::make_optional<Descriptor>(std::move(getter), info);
}
