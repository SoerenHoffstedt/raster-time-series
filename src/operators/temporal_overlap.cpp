
#include "operators/temporal_overlap.h"
#include "datatypes/raster_operations.h"

using namespace rts;

template<class T1, class T2, class T3>
struct TempOverlapCalc {
    static void rasterOperation(TypedRaster<T1> *in1, TypedRaster<T2> *in2, TypedRaster<T3> *out){
        Resolution res = out->getResolution();
        for (int x = 0; x < res.resX; ++x) {
            for (int y = 0; y < res.resY; ++y) {
                T1 val = in1->getCell(x,y) * 1000 + in2->getCell(x,y);
                out->setCell(x, y, (T3)val);
            }
        }
    }
};

TemporalOverlap::TemporalOverlap(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
        : GenericOperator(operator_tree, qrect, params, std::move(in)),
        input1Time(), input2Time(), lastTileOfRaster1(false),
        loadRasterFromCache1(false), loadRasterFromCache2(false),
        expression(params["expression"])
{
    checkInputCount(2);
}

void TemporalOverlap::initialize() {

}

OptionalDescriptor TemporalOverlap::nextDescriptor() {
    //TODO: make sure that the last loaded input descriptors are the ones being used for returning, dont load the next one.
    // should be checkable with the end time.
    OptionalDescriptor input1 = boost::none;
    OptionalDescriptor input2 = boost::none;
    //if last tile was last of its rasters and caching was needed, the raster has to be loaded from cache
    if(loadRasterFromCache1){
        input1 = std::move(descriptorCache1[0]);
        descriptorCache1.erase(descriptorCache1.begin());
        if(descriptorCache1.empty())
            loadRasterFromCache1 = false;
    } else {
        input1 = input_operators[0]->nextDescriptor();
        if(!input1)
            return boost::none;
    }

    if(loadRasterFromCache2){
        input2 = std::move(descriptorCache2[0]);
        descriptorCache2.erase(descriptorCache2.begin());
        if(descriptorCache2.empty())
            loadRasterFromCache2 = false;
    } else {
        input2 = input_operators[1]->nextDescriptor();
        if(!input2)
            return boost::none;
    }

    if(input1Time.t1 < input1->rasterInfo.t1) { //new raster
        //check if there are raster in descriptor cache and use those
        if (!descriptorCache1.empty()) {
            loadRasterFromCache1 = true;
            descriptorCache1.push_back(input1); //input1 is the already the next raster, so put into the cache.
            input1 = std::move(descriptorCache1[0]);
            descriptorCache1.erase(descriptorCache1.begin());
        } else {
            loadRasterFromCache1 = false;
        }
        input1Time = input1->rasterInfo; //set input1Time to new Rasters time.
    }

    if(input2Time.t1 < input2->rasterInfo.t1) { //new raster
        //check if there are raster in descriptor cache and use those
        if (!descriptorCache2.empty()) {
            loadRasterFromCache2 = true;
            descriptorCache2.push_back(input2); //input1 is the already the next raster, so put into the cache.
            input2 = std::move(descriptorCache2[0]);
            descriptorCache2.erase(descriptorCache2.begin());
        } else {
            loadRasterFromCache2 = false;
        }
        input2Time = input2->rasterInfo; //set input1Time to new Rasters time.
    }

    //skip the earlier raster when both raster don't overlap
    while(!input1Time.overlapsWithTemporal(input2Time)){

        if(input1Time.t1 < input2Time.t1){
            if(loadRasterFromCache1){
                input1 = descriptorCache1[descriptorCache1.size() - 1];
                descriptorCache1.clear();
                loadRasterFromCache1 = false;
            } else {
                //TODO: replace this
                //input1 = OperatorUtil::skipCurrentTemporal(*input_operators[0], input1);
            }
            input1Time = input1->rasterInfo;
        } else {
            if(loadRasterFromCache2){
                input2 = descriptorCache2[descriptorCache2.size() - 1];
                descriptorCache2.clear();
                loadRasterFromCache2 = false;
            } else {
                //TODO: replace this
                //input2 = OperatorUtil::skipCurrentTemporal(*input_operators[1], input2);
            }
            input2Time = input2->rasterInfo;
        }

    }

    const bool input1NeedsCaching = input1Time.t2 > input2Time.t2;
    const bool input2NeedsCaching = input2Time.t2 > input1Time.t2;
    if(input1NeedsCaching){ //so tiles of input1 must be cached, because potentially another input2 overlaps with input1
        if(descriptorCache1.capacity() == 0)
            descriptorCache1.reserve(input1->rasterTileCount);
        descriptorCache1.push_back(*input1);
    }
    if(input2NeedsCaching){
        if(descriptorCache2.capacity() == 0)
            descriptorCache2.reserve(input2->rasterTileCount);
        descriptorCache2.push_back(*input2);
    }

    //if a raster is cached and its the last tile of the raster, next time the raster has to be loaded from cache
    if(input1NeedsCaching && descriptorCache1.size() == input1->rasterTileCount)
        loadRasterFromCache1 = true;
    if(input2NeedsCaching && descriptorCache2.size() == input2->rasterTileCount)
        loadRasterFromCache2 = true;

    TemporalReference rasterResultTime = input1Time.getOverlapTemporal(input2Time);

    return createOutput(input1, input2, rasterResultTime);
}

OptionalDescriptor TemporalOverlap::getDescriptor(int tileIndex) {
    auto input1 = input_operators[0]->getDescriptor(tileIndex);
    auto input2 = input_operators[1]->getDescriptor(tileIndex);
    TemporalReference rasterResultTime = input1->rasterInfo.getOverlapTemporal(input2->rasterInfo);
    return createOutput(input1, input2, rasterResultTime);
}

OptionalDescriptor TemporalOverlap::createOutput(OptionalDescriptor &input1, OptionalDescriptor &input2, TemporalReference &rasterResultTime) {
    DescriptorInfo descInfo(input1);
    descInfo.rasterInfo = SpatialTemporalReference(rasterResultTime, input1->rasterInfo, input1->rasterInfo);

    OptionalDescriptorVector inputs;
    inputs.reserve(2);
    inputs.emplace_back(std::move(input1));
    inputs.emplace_back(std::move(input2));

    auto getter = expression.createGetter(std::move(inputs));

    return rts::make_optional<Descriptor>(std::move(getter), descInfo);
}

bool TemporalOverlap::supportsOrder(Order order) const {
    return order == Order::Temporal;
}
