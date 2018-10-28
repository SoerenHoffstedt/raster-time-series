
#include "cumulative_sum.h"

using namespace rts;

CumulativeSum::CumulativeSum(QueryRectangle qrect, Json::Value &params, const std::vector<GenericOperator *> &in)
        : GenericOperator(qrect, params, in), sum(0, 0), lastTileSpat(-9999.0, 9999.0, -9999.0, 9999.0)
{
    checkInputCount(1);
}

OptionalDescriptor CumulativeSum::next() {
    auto input = input_operators[0]->next();

    if(!input)
        return std::nullopt;

    //for first tile, sum.data_length will be 0,or when new tile started: set sum raster to 0s.
    if(sum.getDataLength() == 0 || !lastTileSpat.equalsSpatial(input->tileInfo)) {
        setSumRasterZero(input->tileInfo);
        lastTileTemp = input->tileInfo;
    }

    lastTileSpat = input->tileInfo;
    auto totalRect = input->totalInfo;
    auto tileRect = input->tileInfo;
    //TODO: temporal validity of the accumulated tile? is it from the first tile to the current? I would guess so.
    totalRect.t1 = lastTileTemp.t1;
    tileRect.t1 = lastTileTemp.t1;

    auto getter = [input = std::move(input), sum = &sum](const Descriptor &self) -> UniqueRaster {
        UniqueRaster raster_in = input->getRaster();

        for (int x = 0; x < self.tileInfo.res_x; ++x) {
            for (int y = 0; y < self.tileInfo.res_y; ++y) {
                int val = sum->getCell(x, y) + raster_in->getCell(x, y);
                sum->setCell(x, y, val);
                raster_in->setCell(x, y, val);
            }
        }

        return raster_in;
    };

    return std::make_optional<Descriptor>(
            std::move(getter),
            totalRect,
            tileRect);
}

bool CumulativeSum::supportsOrder(Order order) {
    return order == Order::SpatialTemporal;
}

void CumulativeSum::setSumRasterZero(const QueryRectangle &rect) {
    sum = Raster(rect.res_x, rect.res_y);
    for (int x = 0; x < rect.res_x; ++x) {
        for (int y = 0; y < rect.res_y; ++y) {
            sum.setCell(x, y, 0);
        }
    }
}
