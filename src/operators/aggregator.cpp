
#include <util/raster_calculations.h>
#include "datatypes/raster_operations.h"
#include "util/parsing.h"
#include "aggregator.h"

using namespace rts;
using namespace boost::posix_time;

template<class T1, class T2>
struct AggregatorOperation {
    static void rasterOperation(TypedRaster<T1> *aggregated_out_raster, TypedRaster<T2> *aggregating_in_raster, int rasterCount, double nodata, AggregatorFunction function){
        Resolution tileResolution = aggregated_out_raster->getResolution();
        for(int y = 0; y < tileResolution.resY; ++y){
            for(int x = 0; x < tileResolution.resX; ++x){
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
        : GenericOperator(operator_tree, qrect, params, std::move(in)), hasTimeInterval(false), lastTileIndex(-1), nextDescriptorAfterSkipping(std::nullopt)
{
    checkInputCount(1);
}

void Aggregator::initialize() {
    customDataType = params.isMember("custom_data_type") ? Parsing::parseDataType(params["custom_data_type"].asString()) : GDT_Unknown;
    function = Parsing::parseAggregatorFunction(params["function"].asString());

    hasTimeInterval = params.isMember("time_interval");
    if(hasTimeInterval){
        interval = TimeInterval(params["time_interval"]);
        currTime = from_time_t(static_cast<time_t>(qrect.t1));
    }
}

OptionalDescriptor Aggregator::nextDescriptor() {
    //first descriptor could already be loaded in the skip method and stored in nextDescriptorAfterSkipping
    OptionalDescriptor input;
    if(nextDescriptorAfterSkipping.has_value()) {
        input = std::move(nextDescriptorAfterSkipping);
        nextDescriptorAfterSkipping = std::nullopt;
    } else {
        input = input_operators[0]->nextDescriptor();
    }

    if(input == std::nullopt)
        return std::nullopt;

    int index = input->tileIndex;
    if(index > lastTileIndex){ //new tile started
        currTime = from_time_t(static_cast<time_t>(qrect.t1));
    }
    lastTileIndex = index;

    double aggregateFrom  = static_cast<double>(to_time_t(currTime));
    double aggregateUntil = getNextTimeBorder(true);

    //
    while(aggregateFrom < input->rasterInfo.t1 && aggregateUntil < input->rasterInfo.t1){
        aggregateFrom = aggregateUntil;
        aggregateUntil = getNextTimeBorder(true);
    }

    //
    while(input->rasterInfo.t1 < aggregateFrom){
        input = input_operators[0]->nextDescriptor();

        if(input == std::nullopt)
            return std::nullopt;
    }

    double t1 = aggregateFrom;
    double t2 = aggregateUntil;

    double lastT2 = input->rasterInfo.t2;
    std::vector<OptionalDescriptor> descriptors;
    descriptors.push_back(std::move(input));


    while(lastT2 < aggregateUntil && lastT2 < qrect.t2){
        input = input_operators[0]->nextDescriptor();
        if(input == std::nullopt)
            break;
        if(input->tileIndex > index){
            //TODO: this makes getDescriptor not work reliably on this operator, because the state of the input operator already advanced to the next tile/reset to first raster
            // this is the case when (especially) the sampler makes the time series not continous (it can not know the end time of the raster it skipped without changing the state of the input operator)
            // so fix it at the moment here because it is easier to hack it in here. but its not nice...
            nextDescriptorAfterSkipping = input;
            break;
        }
        lastT2 = input->rasterInfo.t2;
        descriptors.emplace_back(std::move(input));
    }

    return createOutput(descriptors, t1, t2);
}

OptionalDescriptor Aggregator::getDescriptor(int tileIndex) {

    //currTime is the interval start for nextRasterIndex, calc interval start time for rasterIndex from the difference.
    ptime intervalStart = currTime;

    //currTime is the beginning of the next time interval, therefore decrease it once.
    ptime currTimeCopy = currTime;
    interval.decrease(currTimeCopy);
    auto t1 = static_cast<double>(to_time_t(currTimeCopy));
    auto t2 = static_cast<double>(to_time_t(currTime));

    std::vector<OptionalDescriptor> descriptors;

    auto cloneQrect = qrect;
    cloneQrect.t1 = t1;
    cloneQrect.t2 = t2;

    //calc spatial rectangle for tile
    //TODO: calculation does not work for arbitrary raster resolution and bounding box
    // and things are calculated multiple times between both function calls here.
    auto spatInfo = RasterCalculations::tileIndexToSpatialRectangle(qrect, tileIndex);
    Resolution rasterTileCount = RasterCalculations::calculateTileCount(qrect, qrect.projection.getOrigin(), qrect.scale).first;

    if(spatInfo.x1 < qrect.x1)
        spatInfo.x1 = qrect.x1;
    if(spatInfo.x2 > qrect.x2)
        spatInfo.x2 = qrect.x2;
    if(spatInfo.y1 < qrect.y1)
        spatInfo.y1 = qrect.y1;
    if(spatInfo.y2 > qrect.y2)
        spatInfo.y2 = qrect.y2;

    cloneQrect.x1 = spatInfo.x1;
    cloneQrect.y1 = spatInfo.y1;
    cloneQrect.x2 = spatInfo.x2;
    cloneQrect.y2 = spatInfo.y2;
    cloneQrect.resX = static_cast<uint32_t>((cloneQrect.x2 - cloneQrect.x1) / qrect.scale.x);
    cloneQrect.resY = static_cast<uint32_t>((cloneQrect.y2 - cloneQrect.y1) / qrect.scale.y);


    auto clonedOperator = input_operators[0]->reInstantiate(cloneQrect);

    for(auto &inDesc : *clonedOperator){
        if(inDesc.rasterInfo.t1 < t1)
            continue;

        //reset the raster info to the actual whole raster, because the cloned operator got another query.
        inDesc.rasterInfo.x1 = qrect.x1;
        inDesc.rasterInfo.x2 = qrect.x2;
        inDesc.rasterInfo.y1 = qrect.y1;
        inDesc.rasterInfo.y2 = qrect.y2;
        inDesc.rasterInfo.resX = qrect.resX;
        inDesc.rasterInfo.resY = qrect.resY;
        inDesc.tileIndex = tileIndex;
        inDesc.rasterTileCount = rasterTileCount.resX * rasterTileCount.resY;
        inDesc.rasterTileCountDimensional = rasterTileCount;

        descriptors.emplace_back(inDesc);
    }

    return createOutput(descriptors, t1, t2);
}

OptionalDescriptor Aggregator::createOutput(OptionalDescriptorVector &descriptors, double t1, double t2) {

    DescriptorInfo info = descriptors[0].value();
    if(customDataType != GDT_Unknown)
        info.dataType = customDataType;
    info.rasterInfo.t1 = t1;
    info.rasterInfo.t2 = t2;

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

double Aggregator::getNextTimeBorder(bool increaseCurrTime) {
    if(!hasTimeInterval)
        return std::numeric_limits<double>::max();
    else if(increaseCurrTime){
        interval.increase(currTime);
        return (double)to_time_t(currTime);
    } else {
        ptime currTimeCopy = currTime;
        interval.increase(currTimeCopy);
        return (double)to_time_t(currTimeCopy);
    }
}

void Aggregator::skipCurrentRaster(const uint32_t skipCount) {
    if(skipCount == 0)
        return;
    interval.increase(currTime, skipCount - 1); //-1 because the currTime was already incremented once in nextDescriptor.
    double aggregateFrom  = static_cast<double>(to_time_t(currTime));
    double aggregateUntil = getNextTimeBorder(false);
    if(aggregateFrom >= qrect.t2){
        skipCurrentTile();
        return;
    }
    OptionalDescriptor inputDesc = input_operators[0]->nextDescriptor();
    //if t2 of inputDesc is >= aggregateFrom, the next returned raster will be part of the next result.
    while(inputDesc->rasterInfo.t1 < aggregateFrom && inputDesc->rasterInfo.t2 < aggregateFrom){
        input_operators[0]->skipCurrentRaster();
        inputDesc = input_operators[0]->nextDescriptor();
        if(!inputDesc.has_value()){
            return;
        }
    }
    if(inputDesc->rasterInfo.t1 >= aggregateFrom)
        nextDescriptorAfterSkipping = inputDesc;
}
