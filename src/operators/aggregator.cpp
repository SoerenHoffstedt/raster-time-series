
#include "operators/aggregator.h"
#include "datatypes/raster_operations.h"
#include "util/parsing.h"

using namespace rts;

template<class T1, class T2>
struct Aggregating {
    static void rasterOperation(TypedRaster<T1> *addingTo, TypedRaster<T2> *added, int rasterCount, double nodata){
        Resolution tileResolution = addingTo->getResolution();
        for(int x = 0; x < tileResolution.res_x; ++x){
            for(int y = 0; y < tileResolution.res_y; ++y){
                T2 val = added->getCell(x,y);
                if(val == (T2)nodata)
                    addingTo->setCell(x, y, (T2)nodata);
                else {
                    T1 curr_avg = addingTo->getCell(x, y);
                    // A is current avg value after I values, than is the avg with the next value x: (A * I + x) / (I+1)
                    T1 newVal = (curr_avg * (rasterCount) + val) / (rasterCount+1);
                    addingTo->setCell(x,y, newVal);
                }
            }
        }
    }
};

Aggregator::Aggregator(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
        : GenericOperator(qrect, params, std::move(in)), nextDesc(std::nullopt), hasTimeInterval(false)
{
    checkInputCount(1);
    customDataType = params.isMember("custom_data_type") ? Parsing::parseDataType(params["custom_data_type"].asString()) : GDT_Unknown;
}

OptionalDescriptor Aggregator::nextDescriptor() {
    //first descriptor could already be loaded and stored in nextDesc
    OptionalDescriptor input = nextDesc != std::nullopt ? nextDesc : input_operators[0]->nextDescriptor();

    if(input == std::nullopt){
        return std::nullopt;
    }

    int index = input->tileIndex;
    std::vector<OptionalDescriptor> descriptors;
    descriptors.push_back(std::move(input));

    //save all same tile descriptors into the vector. when desc is the next tile, save it into member variable nextDesc
    while(true){
        OptionalDescriptor desc = input_operators[0]->nextDescriptor();

        if(desc == std::nullopt){
            nextDesc = std::nullopt;
            break;
        }
        else if(desc->tileIndex != index){
            nextDesc = std::move(desc);
            break;
        } else {
            descriptors.push_back(std::move(desc));
        }
    }

    DescriptorInfo info = descriptors[0].value();
    if(customDataType != GDT_Unknown)
        info.dataType = customDataType;
    info.rasterInfo.t1 = descriptors[0]->rasterInfo.t1;
    info.rasterInfo.t2 = descriptors[descriptors.size() - 1]->rasterInfo.t2;

    auto getter = [descriptors = std::move(descriptors)](const Descriptor &self) -> UniqueRaster {
        UniqueRaster out_raster = Raster::createRaster(self.dataType, self.tileResolution);
        RasterOperations::callUnary<RasterOperations::AllValuesSetter>(out_raster.get(), 0);

        for(int i = 0; i < descriptors.size(); i++){
            UniqueRaster r = descriptors[i]->getRaster();
            RasterOperations::callBinary<Aggregating>(out_raster.get(), r.get(), i, self.nodata);
        }

        return out_raster;
    };

    return std::make_optional<Descriptor>(std::move(getter), info);
}

bool Aggregator::supportsOrder(Order order) {
    return order == Order::Spatial;
}
