
#include "temporal_overlap.h"

using namespace rts;

TemporalOverlap::TemporalOverlap(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
        : GenericOperator(qrect, params, std::move(in)), input1Time(), input2Time(), lastTileOfRaster1(false), loadRasterFromCache1(false), loadRasterFromCache2(false)
{
    checkInputCount(2);
}

OptionalDescriptor TemporalOverlap::nextDescriptor() {
    OptionalDescriptor input1 = std::nullopt;
    OptionalDescriptor input2 = std::nullopt;
    //if last tile was last of its rasters and caching was needed, the raster has to be loaded from cache
    if(loadRasterFromCache1){
        input1 = std::move(descriptorCache1[0]);
        descriptorCache1.erase(descriptorCache1.begin());
        if(descriptorCache1.empty())
            loadRasterFromCache1 = false;
    } else {
        input1 = input_operators[0]->nextDescriptor();
        if(!input1.has_value())
            return std::nullopt;
    }

    if(loadRasterFromCache2){
        input2 = std::move(descriptorCache2[0]);
        descriptorCache2.erase(descriptorCache2.begin());
        if(descriptorCache2.empty())
            loadRasterFromCache2 = false;
    } else {
        input2 = input_operators[1]->nextDescriptor();
        if(!input2.has_value())
            return std::nullopt;
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
                input1 = OperatorUtil::skipCurrentTemporal(*input_operators[0], input1);
            }
            input1Time = input1->rasterInfo;
        } else {
            if(loadRasterFromCache2){
                input2 = descriptorCache2[descriptorCache2.size() - 1];
                descriptorCache2.clear();
                loadRasterFromCache2 = false;
            } else {
                input2 = OperatorUtil::skipCurrentTemporal(*input_operators[1], input2);
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

    TemporalReference raster_result_time = input1Time.getOverlapTemporal(input2Time);
    DescriptorInfo descInfo(input1);
    descInfo.rasterInfo = SpatialTemporalReference(raster_result_time, input1->rasterInfo, input1->rasterInfo);

    auto getter = [input1 = std::move(input1), input2 = std::move(input2)](const Descriptor &self) -> UniqueRaster {
        UniqueRaster raster_in1 = input1->getRaster();
        UniqueRaster raster_in2 = input2->getRaster();
        UniqueRaster out_raster = std::make_unique<Raster>(self.tileResolution.res_x, self.tileResolution.res_y);

        for (int x = 0; x < self.tileResolution.res_x; ++x) {
            for (int y = 0; y < self.tileResolution.res_y; ++y) {
                int val = raster_in1->getCell(x,y) * 1000 + raster_in2->getCell(x,y);
                out_raster->setCell(x, y, val);
            }
        }

        return out_raster;
    };

    return std::make_optional<Descriptor>(std::move(getter), descInfo);
}

bool TemporalOverlap::supportsOrder(Order order) {
    return order == Order::Temporal;
}
