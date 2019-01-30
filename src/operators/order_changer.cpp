
#include "operators/order_changer.h"

using namespace rts;

OrderChanger::OrderChanger(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
        : GenericOperator(operator_tree, qrect, params, std::move(in)), initialized(false)
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
    currTile = 0;
}

OptionalDescriptor OrderChanger::nextDescriptor() {

    if(!initialized)
    {
        descriptors.reserve(1024);
        for(auto desc : *input_operators[0]){
            desc.order = targetOrder;
            descriptors.emplace_back(std::move(desc));
        }
        if(descriptors.empty())
            return std::nullopt;

        tilesPerRaster = descriptors[0]->rasterTileCount;
        totalTiles = descriptors.size();
        if(totalTiles % tilesPerRaster != 0){
            throw std::runtime_error("Order Changer: number of tiles does not match a multiple of the tiles per raster count.");
        }
        rasterCount = totalTiles / tilesPerRaster;
        initialized = true;
    }


    if(targetOrder == Order::Temporal){

        if(currRaster >= rasterCount)
            return std::nullopt;

        uint64_t index = currRaster + currTile * rasterCount;

        //increase currTile until matches number of tiles per Raster
        //than increase currRaster.

        currTile += 1;
        if(currTile == tilesPerRaster){
            currTile = 0;
            currRaster += 1;
        }

        return descriptors[index];

    } else {

        if(currTile >= tilesPerRaster)
            return std::nullopt;

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

OptionalDescriptor OrderChanger::getDescriptor(int tileSize) {
    //TODO: implement
    return std::nullopt;
}
