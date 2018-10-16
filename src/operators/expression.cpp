
#include "expression.h"

using namespace rts;

Expression::Expression(QueryRectangle qrect, Json::Value &params, const std::vector<GenericOperator *> &in) : GenericOperator(qrect, params, in)
{
    checkInputCount(1);
}

UniqueDescriptor Expression::next() {
    UniqueDescriptor in = input_operators[0]->next();

    if(in == nullptr) {
        return nullptr;
    }

    TemporalReference temp_ref(in->st_ref.t1, in->st_ref.t2);
    SpatialReference spat_ref(in->st_ref.x1, in->st_ref.x2, in->st_ref.y1, in->st_ref.y2);
    Resolution res(in->st_ref.res_x, in->st_ref.res_y);

    auto getter = [in_desc = std::move(in)](Descriptor *self) -> std::unique_ptr<Raster> {
        UniqueRaster test = in_desc->getRaster();
        for (int x = 0; x < self->st_ref.res_x; ++x) {
            for (int y = 0; y < self->st_ref.res_y; ++y) {
                int val = test->getCell(x,y);
                if(val != -1) //TODO: replace with nodata, that has to be put into Descriptor
                    test->setCell(x, y, val * val);
            }
        }
        return test;
    };

    return createUniqueDescriptor(std::move(getter), temp_ref, spat_ref, res, qrect.order);
}

bool Expression::supportsOrder(Order order) {
    return order == Order::TemporalSpatial || order == Order::SpatialTemporal;
}
