
#include "operators/aggregator.h"
#include "datatypes/raster_operations.h"
#include "util/parsing.h"

using namespace rts;
using namespace boost::posix_time;

template<class T1, class T2>
struct AggregatorOperation {
    static void rasterOperation(TypedRaster<T1> *aggregated_out_raster, TypedRaster<T2> *aggregating_in_raster, int rasterCount, double nodata, AggregatorFunction function){
        Resolution tileResolution = aggregated_out_raster->getResolution();
        for(int x = 0; x < tileResolution.res_x; ++x){
            for(int y = 0; y < tileResolution.res_y; ++y){
                T2 val = aggregating_in_raster->getCell(x,y);
                T1 out_old = aggregated_out_raster->getCell(x,y);

                switch(function){
                    case AggregatorFunction::Mean:
                        if(val == (T2)nodata)
                            aggregated_out_raster->setCell(x, y, (T2)nodata);
                        else {
                            T1 curr_avg = aggregated_out_raster->getCell(x, y);
                            // A is current avg value after I values, than is the avg with the next value x: (A * I + x) / (I+1)
                            T1 newVal = (curr_avg * (rasterCount) + val) / (rasterCount+1);
                            aggregated_out_raster->setCell(x,y, newVal);
                        }
                        break;
                    case AggregatorFunction::Min:
                        if(out_old == (T1)nodata || out_old > val)
                            aggregated_out_raster->setCell(x, y, (T1)val);
                        break;
                    case AggregatorFunction::Max:
                        if(out_old == (T1)nodata || out_old < val)
                            aggregated_out_raster->setCell(x, y, (T1)val);
                        break;
                    case AggregatorFunction::Sum:
                        if(val != (T2)nodata){
                            aggregated_out_raster->setCell(x, y, out_old + (T1)val);
                        }
                        break;
                }
            }
        }
    }
};

Aggregator::Aggregator(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
        : GenericOperator(operator_tree, qrect, params, std::move(in)), nextDesc(std::nullopt), hasTimeInterval(false), lastTileIndex(-1)
{
    checkInputCount(1);
    customDataType = params.isMember("custom_data_type") ? Parsing::parseDataType(params["custom_data_type"].asString()) : GDT_Unknown;
    function = Parsing::parseAggregatorFunction(params["function"].asString());

    hasTimeInterval = params.isMember("time_interval");
    if(hasTimeInterval){
        interval = TimeInterval(params["time_interval"]);
        currTime = from_time_t(static_cast<time_t>(qrect.t1));
    }
}

void Aggregator::initialize() {

}

OptionalDescriptor Aggregator::nextDescriptor() {
    //first descriptor could already be loaded and stored in nextDesc
    OptionalDescriptor input = nextDesc != std::nullopt ? nextDesc : input_operators[0]->nextDescriptor();

    if(input == std::nullopt){
        return std::nullopt;
    }

    int index = input->tileIndex;
    if(lastTileIndex < index){
        currTime = from_time_t(static_cast<time_t>(qrect.t1));
    }
    lastTileIndex = index;

    std::vector<OptionalDescriptor> descriptors;
    descriptors.push_back(std::move(input));

    double aggregate_until = getNextTimeBorder();

    //save all same tile descriptors into the vector. when desc is the next tile, save it into member variable nextDesc
    while(true){
        OptionalDescriptor desc = input_operators[0]->nextDescriptor();

        if(desc == std::nullopt || desc->tileIndex != index || desc->rasterInfo.t2 >= aggregate_until){
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

    auto getter = [descriptors = std::move(descriptors), function = function](const Descriptor &self) -> UniqueRaster {
        UniqueRaster out_raster = Raster::createRaster(self.dataType, self.tileResolution);
        if(function == AggregatorFunction::Min)
            RasterOperations::callUnary<RasterOperations::AllValuesSetter>(out_raster.get(), self.nodata);
        else
            RasterOperations::callUnary<RasterOperations::AllValuesSetter>(out_raster.get(), 0);

        for(int i = 0; i < descriptors.size(); i++){
            UniqueRaster r = descriptors[i]->getRaster();
            RasterOperations::callBinary<AggregatorOperation>(out_raster.get(), r.get(), i, self.nodata, function);
        }

        return out_raster;
    };

    return std::make_optional<Descriptor>(std::move(getter), info);
}

bool Aggregator::supportsOrder(Order order) const {
    return order == Order::Spatial;
}

double Aggregator::getNextTimeBorder() {
    if(!hasTimeInterval)
        return std::numeric_limits<double>::max();
    else{
        interval.increase(currTime);
        return (double)to_time_t(currTime);
    }
}
