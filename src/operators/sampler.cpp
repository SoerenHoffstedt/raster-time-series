
#include "operators/sampler.h"

using namespace rts;

Sampler::Sampler(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
        : GenericOperator(operator_tree, qrect, params, std::move(in)), lastSendT1(-1),
          toSkip(params["to_skip"].asUInt()), toReturn(params["to_return"].asUInt())
{
    checkInputCount(1);
}

void Sampler::initialize() {

}

OptionalDescriptor Sampler::nextDescriptor() {

    //if returningCount is bigger that toReturn: this is the last tile to return before skipping.
    if(returningCount >= toReturn){

        //skipCurrentRaster handles the skipping differently based on qrect.order, so no if-case here.
        //state is still in the last tile of the last raster, thus + 1.
        skipCurrentRaster(toSkip + 1);

        returningCount = 0;
    }

    auto currInput = input_operators[0]->nextDescriptor();

    if(!currInput.has_value())
        return currInput;

    //if the input is the next tile in spatial order: reset returning count to zero.
    if(qrect.order == Order::Spatial && currInput->tileIndex > lastSendTileIndex){
        returningCount = 0;
    }

    if(qrect.order == Order::Spatial || currInput->tileIndex == currInput->rasterTileCount - 1){
        returningCount += 1;
    }

    lastSendTileIndex = currInput->tileIndex;
    return currInput;
}

OptionalDescriptor Sampler::getDescriptor(int tileIndex) {
    // The state of input/source operators is changed before calling nextDescriptor, so the state is as for the last
    // returned descriptor. So simply use getDescriptor here.
    // Because only rasters, not single tiles can be skipped, this works fine.
    return input_operators[0]->getDescriptor(tileIndex);
}

bool Sampler::supportsOrder(Order order) const {
    return order == Order::Temporal || order == Order::Spatial;
}
