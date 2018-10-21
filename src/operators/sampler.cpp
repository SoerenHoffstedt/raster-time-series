
#include "sampler.h"

using namespace rts;

Sampler::Sampler(QueryRectangle qrect, Json::Value &params, const std::vector<GenericOperator *> &in)
        : GenericOperator(qrect, params, in), lastSendT1(-1)
{
    checkInputCount(1);

    toSkip = params["to_skip"].asUInt();
}

UniqueDescriptor Sampler::next() {

    auto currInput = input_operators[0]->next();

    if(currInput == nullptr)
        return currInput;

    //TODO: how to handle the ordering?
    // what is skipped? single tile descriptors?
    // or a whole first order dimension, meaning skipping all tiles
    // from one raster if the order is TemporalSpatial? -> this seems logical to me.

    if(qrect.order == Order::TemporalSpatial){
        //send all tiles that have same time as the last send time.
        //TODO: check start and end time?
        if(lastSendT1 != -1 && lastSendT1 == currInput->tileInfo.t1){
            return currInput;
        }

        for(int i = 0; i < toSkip; ++i){
            //skip all tiles of this raster.
            double skipping_t1 = currInput->tileInfo.t1;
            while(true){
                currInput = input_operators[0]->next();
                if(skipping_t1 != currInput->tileInfo.t1)
                    break;
            }
        }

    } else if(qrect.order == Order::SpatialTemporal){

        //TODO: actually has to check if coords of tile are same as before, but coords are not provided rightly by FakeSource atm.
        if(lastSendT1 != -1 && currInput->tileInfo.t1 > lastSendT1){
            return currInput;
        }

        //TODO: and here check if the same coords are skipped instead of time. but see above.
        for(int i = 0; i < toSkip; ++i){
            double last_t1 = currInput->tileInfo.t1;
            while(true){
                currInput = input_operators[0]->next();
                if(currInput->tileInfo.t1 <= last_t1)
                    break;
            }
        }
    }

    //TODO: can i simply return the descriptor I got? Some reproduction of the operator tree from a
    //      descriptor might not be possible this way. But its more performant to skip this.

    lastSendT1 = currInput->tileInfo.t1;
    return currInput;
}

bool Sampler::supportsOrder(Order order) {
    return order == Order::TemporalSpatial || order == Order::SpatialTemporal;
}
