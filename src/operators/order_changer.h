
#ifndef RASTER_TIME_SERIES_ORDER_CHANGER_H
#define RASTER_TIME_SERIES_ORDER_CHANGER_H

#include "generic_operator.h"

namespace rts {

    class OrderChanger : public GenericOperator {
    public:
        OrderChanger(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        OptionalDescriptor nextDescriptor() override;
        OptionalDescriptor getDescriptor(int tileIndex) override;
        void initialize() override;
        bool supportsOrder(Order order) const override;
    private:
        OptionalDescriptor temporalTargetDescriptor;
        std::vector<OptionalDescriptor> descriptors;
        uint32_t tilesPerRaster;
        uint64_t rasterCount;
        int32_t currTile;
        uint32_t currRaster;
        uint64_t totalTiles;
        Order targetOrder;
        Order incomingOrder;
        bool initialized;
        /**
         * Recursively sets the the qrect.order of all input operators of op to order. Does not change the order of op.
         */
        void setOrderOfChildOperators(GenericOperator *op, Order order);
    };

}


#endif //RASTER_TIME_SERIES_ORDER_CHANGER_H
