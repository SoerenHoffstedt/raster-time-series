
#include "sampler.h"

using namespace rts;

Sampler::Sampler(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
        : GenericOperator(qrect, params, std::move(in)), lastSendT1(-1)
{
    checkInputCount(1);

    toSkip = params["to_skip"].asUInt();
}

OptionalDescriptor Sampler::next() {

    auto currInput = input_operators[0]->next();

    if(!currInput.has_value())
        return currInput;

    //TODO: how to handle the ordering?
    // what is skipped? single tile descriptors?
    // or a whole first order dimension, meaning skipping all tiles
    // from one raster if the order is TemporalSpatial? -> this seems logical to me.

    if(qrect.order == Order::TemporalSpatial){
        //send all tiles that have same time as the last send time.
        //TODO: check start and end time?
        if(lastSendT1 != -1 && lastSendT1 == currInput->rasterInfo.t1){
            return currInput;
        }

        for(int i = 0; i < toSkip; ++i){
            //skip all tiles of this raster.
            currInput = OperatorUtil::skipCurrentTemporal(*input_operators[0], currInput);
        }

    } else if(qrect.order == Order::SpatialTemporal){

        //TODO: actually has to check if coords of tile are same as before, but coords are not provided rightly by FakeSource atm.
        if(lastSendT1 != -1 && currInput->rasterInfo.t1 > lastSendT1){
            return currInput;
        }

        //TODO: and here check if the same coords are skipped instead of time. but see above.
        for(int i = 0; i < toSkip; ++i){
           currInput = OperatorUtil::skipCurrentSpatial(*input_operators[0], currInput);
        }
    }

    //TODO: can i simply return the descriptor I got? Some reproduction of the operator tree from a
    //      descriptor might not be possible this way. But its more performant to skip this.
    if(currInput.has_value())
        lastSendT1 = currInput->rasterInfo.t1;

    return currInput;
}

bool Sampler::supportsOrder(Order order) {
    return order == Order::TemporalSpatial || order == Order::SpatialTemporal;
}
