
#include "cumulative_sum.h"
#include "datatypes/raster_operations.h"

using namespace rts;

template<class T1, class T2>
struct CumSumAdder {
    static void rasterOperation(TypedRaster<T1> *sum_raster, TypedRaster<T2> *input_raster, Resolution tileResolution) {
        for (int x = 0; x < tileResolution.res_x; ++x) {
            for (int y = 0; y < tileResolution.res_y; ++y) {
                T1 val = sum_raster->getCell(x, y) + input_raster->getCell(x, y);
                sum_raster->setCell(x, y, val);
                input_raster->setCell(x, y, val);
            }
        }
    }
};

CumulativeSum::CumulativeSum(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
        : GenericOperator(operator_tree, qrect, params, std::move(in)), sum(nullptr), lastTileIndex(-1)
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
        lastTileTemp = input->rasterInfo;
    }

    lastTileIndex = input->tileIndex;
    DescriptorInfo descInfo(input);
    //TODO: temporal validity of the accumulated tile? is it from the first tile to the current? I would guess so.
    descInfo.rasterInfo.t1 = lastTileTemp.t1;

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
