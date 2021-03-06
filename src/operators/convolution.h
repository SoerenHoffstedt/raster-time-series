
#ifndef RASTER_TIME_SERIES_CONVOLUTION_H
#define RASTER_TIME_SERIES_CONVOLUTION_H

#include "operators/generic_operator.h"

namespace rts {

    /**
     * Operator for processing convolution functions on rasters, e.g. edge detection.
     * For now it only supports laplacian edge detection (taken from: http://desktop.arcgis.com/en/arcmap/10.3/manage-data/raster-and-images/convolution-function.htm).
     * Support for different convolution function kernels must be added in the future.
     *
     * The important problem this operator solves is, how to handle accessing multiple adjacent tiles to output one tile.
     * So this operator caches all tile descriptors of a raster into a vector when opening the first tile.
     * Which tile of the currently cached raster is worked on, is determined by the member currTileIndex.
     */
    class Convolution : public GenericOperator {
    public:
        Convolution(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, UniqueOperatorVector &&in);
        OptionalDescriptor nextDescriptor() override;
        OptionalDescriptor getDescriptor(int tileIndex);
        void initialize() override;
        bool supportsOrder(Order order) const override;
    private:
        OptionalDescriptor createOutput(OptionalDescriptor &mainDescriptor, uint32_t mainTileIndex);

        /**
         * Fills the vector neighbours with the neighbouring tile descriptors.
         * The indexes in neighbours are fixed as follows:
         * 8  1  2
         * 7  0  3
         * 6  5  4
         * index 0: the tile itself.
         * index 1: Above the tile, than go clockwise.
         * index 3: right from the tile, etc...
         *
         * @param neighbours Reference to vector of OptionalDescriptors to be filled with the neighbours of the tile.
         * @param tileIndex The current center tile index. On dim. index from the descriptor.
         */
        void fillWithNeighbourTiles(OptionalDescriptorVector &neighbours, int tileIndex, Resolution tileCountDimensional) const;
        /**
         * Checks if a two dimensional tile index is in bounds of the raster.
         * @param x x index position of the tile in the raster.
         * @param y y index position of the tile in the raster.
         * @return If x,y is a valid two dimensional index for a tile in a raster.
         */
        bool isInRange(int x, int y, Resolution tileCountDimensional) const;
    };

}

#endif //RASTER_TIME_SERIES_CONVOLUTION_H
