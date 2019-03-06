
#include "operators/order_changer.h"

using namespace rts;

OrderChanger::OrderChanger(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
        : GenericOperator(operator_tree, qrect, params, std::move(in)), initialized(false), temporalTargetDescriptor(boost::none)
{
    checkInputCount(1);
}

void OrderChanger::initialize() {
    //The qrect contains the order this operator should produce the tiles in.
    //So change the incoming order to the opposite and set it recursively for
    //the rest of the operator tree.
    targetOrder = qrect.order;
    incomingOrder = targetOrder == Order::Temporal ? Order::Spatial : Order::Temporal;
    setOrderOfChildOperators(this, incomingOrder);
    currRaster = 0;
    currTile = -1;
}

OptionalDescriptor OrderChanger::nextDescriptor() {

    //new version: using random tile access for spatial -> temporal
    if(targetOrder == Order::Temporal){

        currTile += 1;
        if(temporalTargetDescriptor && currTile >= temporalTargetDescriptor->rasterTileCount){
            if(temporalTargetDescriptor->rasterInfo.t2 >= qrect.t2){
                return boost::none;
            }
            input_operators[0]->skipCurrentRaster();
            temporalTargetDescriptor = boost::none;
            currTile = 0;
            currRaster += 1;
        }

        if(currTile == 0){
            temporalTargetDescriptor = input_operators[0]->nextDescriptor();
            if(!temporalTargetDescriptor || temporalTargetDescriptor->tileIndex > 0)
                return boost::none;
        }

        auto desc = (currTile == 0) ? temporalTargetDescriptor : input_operators[0]->getDescriptor(currTile);

        desc->order = targetOrder;

        return desc;

    } else {

        //Temporal to Spatial is not changed, still load into cache
        if(!initialized)
        {
            descriptors.reserve(1024);
            for(auto desc : *input_operators[0]){
                desc.order = targetOrder;
                descriptors.emplace_back(std::move(desc));
            }
            if(descriptors.empty())
                return boost::none;

            tilesPerRaster = descriptors[0]->rasterTileCount;
            totalTiles = descriptors.size();
            if(totalTiles % tilesPerRaster != 0){
                throw std::runtime_error("Order Changer: number of tiles does not match a multiple of the tiles per raster count.");
            }
            rasterCount = totalTiles / tilesPerRaster;
            initialized = true;
        }

        if(currTile >= tilesPerRaster)
            return boost::none;

        uint64_t index = currTile + currRaster * tilesPerRaster;

        currRaster += 1;
        if(currRaster == rasterCount){
            currRaster = 0;
            currTile += 1;
        }

        return descriptors[index];

    }
}

bool OrderChanger::supportsOrder(Order order) const {
    return true;
}

void OrderChanger::setOrderOfChildOperators(GenericOperator *op, Order order) {
    //change the order of all the child operators recursively. Even when it is a order changer again, just change it
    //and it will change it back again.
    for(auto &childOp : op->input_operators){
        childOp->qrect.order = order;
        setOrderOfChildOperators(childOp.get(), order);
    }
}

OptionalDescriptor OrderChanger::getDescriptor(int tileIndex) {
    if(targetOrder == Order::Temporal){
        return input_operators[0]->getDescriptor(tileIndex);
    } else {
        uint64_t index = tileIndex + currRaster * tilesPerRaster;
        return descriptors[index];
    }
}
