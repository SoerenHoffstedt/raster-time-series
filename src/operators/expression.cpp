
#include "expression.h"

using namespace rts;

Expression::Expression(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in) : GenericOperator(qrect, params, std::move(in))
{
    checkInputCount(1);
}

OptionalDescriptor Expression::next() {
    OptionalDescriptor in = input_operators[0]->next();

    if(!in) {
        return std::nullopt;
    }

    Resolution tileRes(in->tileResolution);
    SpatialTemporalReference rasterInfo = in->rasterInfo;
    int tileIndex = in->tileIndex;

    auto getter = [in_desc = std::move(in)](const Descriptor &self) -> std::unique_ptr<Raster> {
        UniqueRaster test = in_desc->getRaster();
        for (int x = 0; x < self.tileResolution.res_x; ++x) {
            for (int y = 0; y < self.tileResolution.res_y; ++y) {
                int val = test->getCell(x,y);
                if(val != -1) //TODO: replace with nodata, that has to be put into Descriptor
                    test->setCell(x, y, val * val);
            }
        }
        return test;
    };

    return std::make_optional<Descriptor>(std::move(getter), rasterInfo, tileRes, qrect.order, tileIndex);
}

bool Expression::supportsOrder(Order order) {
    return order == Order::TemporalSpatial || order == Order::SpatialTemporal;
}
