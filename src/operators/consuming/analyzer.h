
#ifndef RASTER_TIME_SERIES_ANALYZER_H
#define RASTER_TIME_SERIES_ANALYZER_H

#include "operators/consuming/consuming_operator.h"

namespace rts {


    /**
     * Consuming operator that analyzes the minimum, maximum, and average cell values for each raster of a time series.
     * The results are output into a csv file.
     *
     * Parameters:
     *  filename: string name for the output file.
     */
    class Analyzer : public ConsumingOperator {
    public:
        explicit Analyzer (const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        void consume() override;
        void initialize() override;
        bool supportsOrder(Order o) const override;
    };

    class RasterStats {
    public:
        RasterStats(int index) : index(index) {
            cellNum = 0;
            average = 0;
            maximum = std::numeric_limits<double>::min();
            minimum = std::numeric_limits<double>::max();
        }
        int index;
        double cellNum;
        double maximum;
        double minimum;
        double average;
    };


}


#endif //RASTER_TIME_SERIES_ANALYZER_H
