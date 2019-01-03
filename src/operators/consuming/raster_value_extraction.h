
#ifndef RASTER_TIME_SERIES_RASTER_VALUE_EXTRACTION_H
#define RASTER_TIME_SERIES_RASTER_VALUE_EXTRACTION_H

#include "operators/consuming/consuming_operator.h"

namespace rts {

    /**
     * Information about a point for the raster value extraction.
     * Contains of a coordinate (x,y) and a time t.
     * Additionally on creation the index for the tile that contains (x,y) is calculated for sorting the points.
     */
    class TemporalPoint {
    public:
        TemporalPoint(const Json::Value &def, SpatialReference &tileSizeSpat, SpatialReference &extent);
        TemporalPoint(double x, double y, double t, SpatialReference &tileSizeSpat, SpatialReference &extent);
        double x;
        double y;
        double t;
    private:
        void calcAndSetTileIndex(SpatialReference &tileSizeSpat, SpatialReference &extent);

    public:
        /**
         * The index of the tile this point belongs to spatially for the given query rectangle.
         */
        uint32_t tileIndex;

        /**
         *
         * @param p1
         * @param p2
         * @return True if p1 must be ordered before p2.
         */
        static bool comparerTemporal(const TemporalPoint &p1, const TemporalPoint &p2);
        static bool comparerSpatial(const TemporalPoint &p1, const TemporalPoint &p2);
    };

    /**
     * Operator for extracting pixel values at specific points.
     * Params:
     *      - points: a json array containing different point objects, each containing coordinates (x,y) and time t.
     *      - output: print or file.
     *      - filename: string for the output filename if output == "file".
     */
    class RasterValueExtraction : public ConsumingOperator {
    public:
        RasterValueExtraction(QueryRectangle qrect, Json::Value &params, UniqueOperatorVector &&in);
        void consume() override;
        bool supportsOrder(Order order) const override;
    private:
        void printPixelAt(std::ostream &output, Raster *raster, int x, int y) const;
        enum class Output {
            Print,
            File
        };
        Output output;
    };

}

#endif //RASTER_TIME_SERIES_RASTER_VALUE_EXTRACTION_H
