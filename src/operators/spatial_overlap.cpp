
#include "operators/spatial_overlap.h"

using namespace rts;

rts::SpatialOverlap::SpatialOverlap(rts::QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
        : GenericOperator(qrect, params, std::move(in))
{
    checkInputCount(2);
}

OptionalDescriptor SpatialOverlap::next() {

    auto input1 = input_operators[0]->next();
    auto input2 = input_operators[1]->next();

    while(true){

        if(!input1.has_value() || !input2.has_value())
            return std::nullopt;

        SpatialReference overlapRect = input1->rasterInfo.getOverlapSpatial(input2->rasterInfo);
        TemporalReference temp_total = input1->rasterInfo;
        TemporalReference input1_temp = input1->rasterInfo;
        //TemporalReference input2_temp = input2->rasterInfo;

        //TODO: only use the time both are viable
        if(input2->rasterInfo.t1 < input1->rasterInfo.t1)
            temp_total.t1 = input2->rasterInfo.t1;
        if(input2->rasterInfo.t2 > input1->rasterInfo.t2)
            temp_total.t2 = input2->rasterInfo.t2;

        QueryRectangle totalInfo(temp_total, overlapRect, Resolution(qrect.res_x, qrect.res_y), Order::TemporalSpatial);
        Resolution tileResolution = input1->tileResolution;
        int tileIndex = input1->tileIndex;

        SpatialReference tileCoords = input1->calculateCoordinatesOfTile();

        while(!tileCoords.overlapsWithSpatial(overlapRect)) {
            input1 = input_operators[0]->next();
            input2 = input_operators[1]->next();
            if(input1->rasterInfo.t1 > input1_temp.t1) //new raster
                continue;
            tileCoords = input1->calculateCoordinatesOfTile();
        }
        if(input1->rasterInfo.t1 > input1_temp.t1) //new raster
            continue;

        //TODO: returning whole tiles at the moment, so its more data than the overlap rect promises. Fill with nodata, or make the overlap rect bigger.
        auto getter = [input1 = std::move(input1), input2 = std::move(input2)](const Descriptor &self) -> UniqueRaster {
            UniqueRaster raster_in1 = input1->getRaster();
            UniqueRaster raster_in2 = input2->getRaster();
            UniqueRaster out_raster = std::make_unique<Raster>(self.tileResolution);

            for (int x = 0; x < self.tileResolution.res_x; ++x) {
                for (int y = 0; y < self.tileResolution.res_y; ++y) {
                    int val = (raster_in1->getCell(x,y) + raster_in2->getCell(x,y)) / 2;
                    out_raster->setCell(x, y, val);
                }
            }

            return out_raster;
        };

        return std::make_optional<Descriptor>(std::move(getter), totalInfo, tileResolution, Order::TemporalSpatial, tileIndex);
    }
}

bool rts::SpatialOverlap::supportsOrder(Order order) {
    return order == Order::TemporalSpatial;
}
