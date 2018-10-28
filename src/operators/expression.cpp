
#include "expression.h"

using namespace rts;

Expression::Expression(QueryRectangle qrect, Json::Value &params, const std::vector<GenericOperator *> &in) : GenericOperator(qrect, params, in)
{
    checkInputCount(1);
}

OptionalDescriptor Expression::next() {
    OptionalDescriptor in = input_operators[0]->next();

    if(!in) {
        return std::nullopt;
    }

    TemporalReference temp_ref(in->tileInfo.t1, in->tileInfo.t2);
    SpatialReference spat_ref(in->tileInfo.x1, in->tileInfo.x2, in->tileInfo.y1, in->tileInfo.y2);
    Resolution res(in->tileInfo.res_x, in->tileInfo.res_y);

    QueryRectangle infoTotal = in->totalInfo;

    auto getter = [in_desc = std::move(in)](const Descriptor &self) -> std::unique_ptr<Raster> {
        UniqueRaster test = in_desc->getRaster();
        for (int x = 0; x < self.tileInfo.res_x; ++x) {
            for (int y = 0; y < self.tileInfo.res_y; ++y) {
                int val = test->getCell(x,y);
                if(val != -1) //TODO: replace with nodata, that has to be put into Descriptor
                    test->setCell(x, y, val * val);
            }
        }
        return test;
    };

    return std::make_optional<Descriptor>(
            std::move(getter),
            infoTotal,
            QueryRectangle(temp_ref, spat_ref, res, qrect.order));
}

bool Expression::supportsOrder(Order order) {
    return order == Order::TemporalSpatial || order == Order::SpatialTemporal;
}
