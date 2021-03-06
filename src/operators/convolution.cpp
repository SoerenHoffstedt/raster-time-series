
#include "operators/convolution.h"
#include "datatypes/raster.h"
#include "datatypes/raster_operations.h"

using namespace rts;

enum class DirectionEight {
    Up = 1,
    UpRight = 2,
    Right = 3,
    DownRight = 4,
    Down = 5,
    DownLeft = 6,
    Left = 7,
    UpLeft = 8
};

/**
 * Proving rasterOperation function for the convolution operator. Only providing
 */
template <typename T1, typename T2>
struct ConvolutionOperation {

    /**
     * Returning the tile index for the neighbour tiles by providing a delta for x and y dimension. dx,dy should
     * only be -1,0,1. The tile itself here is index 0 and with dx,dy the following index will be returned:
     * 8  1  2
     * 7  0  3
     * 6  5  4
     */
    static int getIndex(int dx, int dy) {
        if(dx == 0 && dy == 0)
            return 0;
        if(dx == 0 && dy == -1)
            return 1;
        if(dx == 1 && dy == -1)
            return 2;
        if(dx == 1 && dy == 0)
            return 3;
        if(dx == 1 && dy == 1)
            return 4;
        if(dx == 0 && dy == 1)
            return 5;
        if(dx == -1 && dy == 1)
            return 6;
        if(dx == -1 && dy == 0)
            return 7;
        if(dx == -1 && dy == -1)
            return 8;

        throw std::runtime_error("delta x and y should only be -1,0,1.");
    }

    /**
     * Function returning the value of the cell (x,y) of the tile (center). It is handling the case that (x,y) is out
     * of bounds of the tile by getting the value from an adjacent raster (all).
     */
    static double getCell(int x, int y, Resolution &res, TypedRaster<T2> *center, std::vector<TypedRaster<T2>*> &all){
        int dx = 0;
        int dy = 0;

        if(x < 0)
            dx = -1;
        else if(x >= res.resX)
            dx = 1;
        if(y < 0)
            dy = -1;
        else if(y >= res.resY)
            dy = 1;

        if(dx == 0 && dy == 0)
            return static_cast<double>(center->getCell(x,y));

        int index = getIndex(dx, dy);
        if(all[index] == nullptr)
            return 0.0;

        if(dx != 0)
            x -= dx * res.resX;
        if(dy != 0)
            y -= dy * res.resY;
        return static_cast<double>(all[index]->getCell(x,y));

    }

    static void rasterOperation(TypedRaster<T1> *out_raster, TypedRaster<T2> *input_center, std::vector<Raster*> &in_raster) {

        std::vector<TypedRaster<T2>*> in_raster_casted;
        in_raster_casted.reserve(9);
        for (int i = 0; i < 9; ++i) {
            in_raster_casted.push_back((TypedRaster<T2>*)in_raster[i]);
        }

        int conv_length = 3;
        int skip = conv_length / 2;

        Resolution res = out_raster->getResolution();

        for (int y = 0; y < res.resY; ++y) {
            for (int x = 0; x < res.resX; ++x) {

                //using laplacian edge detection for now. TODO: Make others usable by passing a kernel.
                //taken from: http://desktop.arcgis.com/en/arcmap/10.3/manage-data/raster-and-images/convolution-function.htm
                double out_val = 4.0 * getCell(x,y, res, input_center, in_raster_casted); // input_center->getCell(x,y);

                out_val += -1.0 * getCell(x+1,y-1, res, input_center, in_raster_casted); //input_center->getCell(x+1,y-1);
                out_val += -1.0 * getCell(x+1,y+1, res, input_center, in_raster_casted); //input_center->getCell(x+1,y+1);
                out_val += -1.0 * getCell(x-1,y-1, res, input_center, in_raster_casted); //input_center->getCell(x-1,y-1);
                out_val += -1.0 * getCell(x-1,y+1, res, input_center, in_raster_casted); //input_center->getCell(x-1,y+1);

                if(!std::numeric_limits<T1>::is_signed && out_val < 0.0)
                    out_val *= -1;

                T1 out_casted = 0;
                if(out_val < std::numeric_limits<T1>::min())
                    out_casted = std::numeric_limits<T1>::min();
                else if(out_val > std::numeric_limits<T1>::max())
                    out_casted = std::numeric_limits<T1>::max();
                else
                    out_casted = static_cast<T1>(out_val);

                out_raster->setCell(x, y, out_casted);
            }
        }

    }
};

Convolution::Convolution(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, UniqueOperatorVector &&in)
        : GenericOperator(operator_tree, qrect, params, std::move(in))
{
    checkInputCount(1);
}

void Convolution::initialize() {

}

OptionalDescriptor Convolution::nextDescriptor() {

    auto input = input_operators[0]->nextDescriptor();

    if(!input)
        return boost::none;

    auto output = createOutput(input, input->tileIndex);

    return output;
}

OptionalDescriptor Convolution::getDescriptor(int tileIndex) {
    auto mainDescriptor = input_operators[0]->getDescriptor(tileIndex);

    if(!mainDescriptor)
        return boost::none;

    return createOutput(mainDescriptor, tileIndex);
}

OptionalDescriptor Convolution::createOutput(OptionalDescriptor &mainDescriptor, uint32_t mainTileIndex) {

    OptionalDescriptorVector neighbours;
    neighbours.reserve(9);
    neighbours.push_back(std::move(mainDescriptor));
    fillWithNeighbourTiles(neighbours, mainTileIndex, neighbours[0]->rasterTileCountDimensional);

    DescriptorInfo info = neighbours[0].value();

    auto getter = [neighbours = std::move(neighbours)](const Descriptor &self) mutable -> UniqueRaster {
        auto out_raster = Raster::createRaster(self.dataType, self.tileResolution);

        //save rasters once as vector of UniqueRaster to make sure they will get deleted, and once as raw pointer vector for usage.
        std::vector<Raster*> inputs;
        std::vector<UniqueRaster> in_raster;
        inputs.reserve(9);
        in_raster.reserve(9);

        for(int i = 0; i < 9; i++){
            if(neighbours[i]){
                in_raster.push_back(neighbours[i]->getRaster());
                inputs.push_back(in_raster[i].get());
            } else {
                in_raster.emplace_back(nullptr);
                inputs.emplace_back(nullptr);
            }
        }

        RasterOperations::callBinary<ConvolutionOperation>(out_raster.get(), inputs[0], inputs);

        return out_raster;
    };

    return rts::make_optional<Descriptor>(std::move(getter), info);
}

bool Convolution::supportsOrder(Order order) const {
    return order == Order::Temporal;
}

bool Convolution::isInRange(int x, int y, Resolution tileCountDimensional) const {
    return x >= 0 && y >= 0 && x < tileCountDimensional.resX && y < tileCountDimensional.resY;
}

void Convolution::fillWithNeighbourTiles(OptionalDescriptorVector &neighbours, int tileIndex, Resolution tileCountDimensional) const {
    //tile 0: the tile itself
    //tile 1: Above the tile, than go clockwise.
    //tile 3: right from the tile, etc...
    /* 8  1  2
     * 7  0  3
     * 6  5  4
     */
    //if a tile does not exist, it will be nullopt.

    int indexX = tileIndex % tileCountDimensional.resX;
    int indexY = tileIndex / tileCountDimensional.resX;

    for(int i = 1; i < 9; i++){
        auto dir = static_cast<DirectionEight>(i);
        int x = indexX;
        int y = indexY;
        //change x,y index based on neighbours direction.
        switch(dir){
            case DirectionEight::Up:
            case DirectionEight::UpRight:
            case DirectionEight::UpLeft:
                --y;
                break;
            case DirectionEight::Down:
            case DirectionEight::DownRight:
            case DirectionEight::DownLeft:
                ++y;
                break;
            case DirectionEight::Right:
            case DirectionEight::Left:
                break;
        }
        switch(dir){
            case DirectionEight::Up:
            case DirectionEight::Down:
                break;
            case DirectionEight::UpRight:
            case DirectionEight::Right:
            case DirectionEight::DownRight:
                ++x;
                break;
            case DirectionEight::DownLeft:
            case DirectionEight::Left:
            case DirectionEight::UpLeft:
                --x;
                break;
        }

        //if (x,y) is in range at its tile into neighbours, else add a nullopt.
        if(isInRange(x, y, tileCountDimensional)){
            int index = x + y * tileCountDimensional.resX;
            neighbours.push_back(input_operators[0]->getDescriptor(index));
        } else {
            neighbours.emplace_back(boost::none);

        }
    }

}
