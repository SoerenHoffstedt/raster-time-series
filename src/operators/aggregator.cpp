
#include "aggregator.h"

using namespace rts;

Aggregator::Aggregator(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
        : GenericOperator(qrect, params, std::move(in))
{
    checkInputCount(1);
}

OptionalDescriptor Aggregator::next() {

    std::vector<OptionalDescriptor> descriptors;
    for(OptionalDescriptor input = input_operators[0]->next(); input.has_value(); input = input_operators[0]->next()){
        descriptors.push_back(input);
    }

    if(descriptors.empty()){
        return std::nullopt;
    }

    QueryRectangle tileInfo = descriptors[0]->tileInfo;

    //out raster size, right now just take the tile size.
    int size_x = 0;
    int size_y = 0;
    if(!descriptors.empty()){
        size_x = descriptors[0]->tileInfo.res_x;
        size_y = descriptors[0]->tileInfo.res_y;
    }

    auto getter = [descriptors = std::move(descriptors), size_x = size_x, size_y = size_y](const Descriptor &self) -> UniqueRaster {
        UniqueRaster out_raster = std::make_unique<Raster>(descriptors[0]->tileInfo.res_x, descriptors[0]->tileInfo.res_y);

        for(int i = 0; i < descriptors.size(); i++){
            UniqueRaster r = descriptors[i]->getRaster();

            for(int x = 0; x < self.tileInfo.res_x; ++x){
                for(int y = 0; y < self.tileInfo.res_y; ++y){
                    int val = r->getCell(x,y);
                    int curr_avg = out_raster->getCell(x, y);
                    // A is current avg value after T values, than is the avg with the next value x: (A * T + x) / (T+1)
                    int newVal = (curr_avg * (i) + val) / (i+1);

                    out_raster->setCell(x,y, newVal);
                }
            }

        }

        return out_raster;
    };


    return std::make_optional<Descriptor>(
            std::move(getter),
            qrect, //the aggregated rasters validity is the same as the whole qrect
            tileInfo);
}

bool Aggregator::supportsOrder(Order order) {
    return order == Order::TemporalSpatial || order == Order::SpatialTemporal;
}
