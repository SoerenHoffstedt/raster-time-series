
#include "cumulative_sum.h"

using namespace rts;

CumulativeSum::CumulativeSum(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
        : GenericOperator(qrect, params, std::move(in)), sum(0, 0), lastTileIndex(-1)
{
    checkInputCount(1);
}

OptionalDescriptor CumulativeSum::next() {
    auto input = input_operators[0]->next();

    if(!input)
        return std::nullopt;

    //for first tile, sum.data_length will be 0,or when new tile started: set sum raster to 0s.
    if(sum.getDataLength() == 0 || lastTileIndex != input->tileIndex) {
        setSumRasterZero(input->tileResolution);
        lastTileTemp = input->rasterInfo;
    }

    lastTileIndex = input->tileIndex;
    int tileIndex = input->tileIndex;
    int nodata = input->nodata;
    auto totalRect = input->rasterInfo;
    auto tileRes = input->tileResolution;
    //TODO: temporal validity of the accumulated tile? is it from the first tile to the current? I would guess so.
    totalRect.t1 = lastTileTemp.t1;

    auto getter = [input = std::move(input), sum = &sum](const Descriptor &self) -> UniqueRaster {
        UniqueRaster raster_in = input->getRaster();

        for (int x = 0; x < self.tileResolution.res_x; ++x) {
            for (int y = 0; y < self.tileResolution.res_y; ++y) {
                int val = sum->getCell(x, y) + raster_in->getCell(x, y);
                sum->setCell(x, y, val);
                raster_in->setCell(x, y, val);
            }
        }

        return raster_in;
    };

    return std::make_optional<Descriptor>(std::move(getter), totalRect, tileRes, qrect.order, tileIndex, nodata);
}

bool CumulativeSum::supportsOrder(Order order) {
    return order == Order::SpatialTemporal;
}

void CumulativeSum::setSumRasterZero(const Resolution &res) {
    sum = Raster(res);
    for (int x = 0; x < res.res_x; ++x) {
        for (int y = 0; y < res.res_y; ++y) {
            sum.setCell(x, y, 0);
        }
    }
}
