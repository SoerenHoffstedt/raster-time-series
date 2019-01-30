
#include "operators/cumulative_sum.h"
#include "datatypes/raster_operations.h"
#include "util/raster_calculations.h"

using namespace rts;

template<class T1, class T2>
struct CumSumAdder {
    static void rasterOperation(TypedRaster<T1> *sum_raster, TypedRaster<T2> *input_raster, Resolution tileResolution) {
        for (int x = 0; x < tileResolution.resX; ++x) {
            for (int y = 0; y < tileResolution.resY; ++y) {
                T1 val = sum_raster->getCell(x, y) + input_raster->getCell(x, y);
                sum_raster->setCell(x, y, val);
                input_raster->setCell(x, y, val);
            }
        }
    }
};

CumulativeSum::CumulativeSum(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
        : GenericOperator(operator_tree, qrect, params, std::move(in)), sum(nullptr), lastTileIndex(-1), lastTileT2(0)
{
    checkInputCount(1);
}

void CumulativeSum::initialize() {

}

OptionalDescriptor CumulativeSum::nextDescriptor() {
    auto input = input_operators[0]->nextDescriptor();

    if(!input)
        return std::nullopt;

    if(sum == nullptr){
        sum = Raster::createRaster(input->dataType, input->tileResolution);
    }

    //for first tile, sum.data_length will be 0,or when new tile started: set sum raster to 0s.
    if(sum->getDataLength() == 0 || lastTileIndex != input->tileIndex) {
        setSumRasterZero(input->tileResolution);
        firstTileTemp = input->rasterInfo;
    }

    lastTileIndex = input->tileIndex;
    DescriptorInfo descInfo(input);
    //temporal validity of the accumulated tile is set to the validity of the last input tile
    //this is to keep the cumulated tiles distinguishable.
    lastTileT2 = descInfo.rasterInfo.t2;

    auto getter = [input = std::move(input), sum = sum.get()](const Descriptor &self) -> UniqueRaster {
        UniqueRaster raster_in = input->getRaster();
        RasterOperations::callBinary<CumSumAdder>(sum, raster_in.get(), self.tileResolution);
        return raster_in;
    };

    return std::make_optional<Descriptor>(std::move(getter), descInfo);
}

bool CumulativeSum::supportsOrder(Order order) const {
    return order == Order::Spatial;
}

void CumulativeSum::setSumRasterZero(const Resolution &res) {
    RasterOperations::callUnary<RasterOperations::AllValuesSetter>(sum.get(), 0);
}

OptionalDescriptor CumulativeSum::getDescriptor(int tileIndex) {
    //actually has to reload tile from all rasters of the time series

    double t1 = qrect.t1;
    double t2 = lastTileT2;

    std::vector<OptionalDescriptor> descriptors;

    Resolution tileRes = qrect.tileRes;
    Resolution tileCount;

    auto cloneQrect = qrect;
    cloneQrect.t1 = t1;
    cloneQrect.t2 = t2;
    //calc spatial rectangle for tile
    auto spatInfo = RasterCalculations::tileIndexToSpatialRectangle(qrect, tileIndex);

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
        //it can be that the first raster still overlaps with t1,t2 but started before t1
        // and must therefore be part of the previous output raster.
        if(inDesc.rasterInfo.t1 < t1 || inDesc.rasterInfo.t1 >= t2)
            continue;

        inDesc.rasterInfo.x1 = qrect.x1;
        inDesc.rasterInfo.x2 = qrect.x2;
        inDesc.rasterInfo.y1 = qrect.y1;
        inDesc.rasterInfo.y2 = qrect.y2;
        inDesc.rasterInfo.resX = qrect.resX;
        inDesc.rasterInfo.resY = qrect.resY;
        inDesc.tileIndex = tileIndex;

        //TODO: set these:
        //inDesc.rasterTileCount;
        //inDesc.rasterTileCountDimensional;

        descriptors.emplace_back(inDesc);
    }

    if(descriptors.empty())
        return std::nullopt;

    DescriptorInfo descInfo(qrect, spatInfo, tileRes, Order::Spatial, tileIndex, tileCount, descriptors[0]->nodata, descriptors[0]->dataType);

    //set temp. info to last raster.
    descInfo.rasterInfo.t1 = descriptors[descriptors.size() - 1]->rasterInfo.t1;
    descInfo.rasterInfo.t2 = descriptors[descriptors.size() - 1]->rasterInfo.t2;

    auto getter = [descriptors = std::move(descriptors)](const Descriptor &self) -> UniqueRaster {
        auto sum = Raster::createRaster(self.dataType, self.tileResolution);
        RasterOperations::callUnary<RasterOperations::AllValuesSetter>(sum.get(), 0);
        for(auto &desc : descriptors){
            UniqueRaster raster_in = desc->getRaster();
            RasterOperations::callBinary<CumSumAdder>(sum.get(), raster_in.get(), self.tileResolution);
        }
        return sum;
    };

    return std::make_optional<Descriptor>(std::move(getter), descInfo);
}
