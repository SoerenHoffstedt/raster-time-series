
#include "aggregator.h"
#include "datatypes/raster_operations.h"

using namespace rts;

template<class T1, class T2>
struct Aggregating {
    static void rasterOperation(TypedRaster<T1> *addingTo, TypedRaster<T2> *added, int rasterCount){
        Resolution tileResolution = addingTo->getResolution();
        for(int x = 0; x < tileResolution.res_x; ++x){
            for(int y = 0; y < tileResolution.res_y; ++y){
                T1 curr_avg = addingTo->getCell(x, y);
                T2 val = added->getCell(x,y);
                // A is current avg value after I values, than is the avg with the next value x: (A * I + x) / (I+1)
                T1 newVal = (curr_avg * (rasterCount) + val) / (rasterCount+1);
                addingTo->setCell(x,y, newVal);
            }
        }
    }
};

Aggregator::Aggregator(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
        : GenericOperator(qrect, params, std::move(in))
{
    checkInputCount(1);
}

OptionalDescriptor Aggregator::nextDescriptor() {
    //TODO: make it work tile based, right now it takes simply all tiles.
    std::vector<OptionalDescriptor> descriptors;
    for(OptionalDescriptor input = input_operators[0]->nextDescriptor(); input.has_value(); input = input_operators[0]->nextDescriptor()){
        descriptors.push_back(input);
    }

    if(descriptors.empty()){
        return std::nullopt;
    }

    //out raster size, right now just take the tile size.
    uint32_t size_x = descriptors[0]->tileResolution.res_x;
    uint32_t size_y = descriptors[0]->tileResolution.res_y;
    SpatialReference tileSpatialInfo = descriptors[0]->tileSpatialInfo;
    int tileIndex = descriptors[0]->tileIndex;
    int tileCount = descriptors[0]->rasterTileCount;
    double nodata = descriptors[0]->nodata;
    auto dataType = descriptors[0]->dataType;

    auto getter = [descriptors = std::move(descriptors)](const Descriptor &self) -> UniqueRaster {
        UniqueRaster out_raster = Raster::createRaster(self.dataType, self.tileResolution);

        for(int i = 0; i < descriptors.size(); i++){
            UniqueRaster r = descriptors[i]->getRaster();
            RasterOperations::callBinary<Aggregating>(out_raster.get(), r.get(), i);
        }

        return out_raster;
    };


    return std::make_optional<Descriptor>(std::move(getter), qrect, tileSpatialInfo, Resolution(size_x, size_y), qrect.order, tileIndex, tileCount, nodata, dataType);
}

bool Aggregator::supportsOrder(Order order) {
    return order == Order::Spatial;
}
