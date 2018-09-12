
#ifndef RASTER_TIME_SERIES_EXPRESSION_H
#define RASTER_TIME_SERIES_EXPRESSION_H

#include "operators/generic_operator.h"

namespace rts {


    /**
     * Operator for expression operations between two rasters or one raster with
     */
    class ExpressionOperator : public GenericOperator {
    public:
        enum class Operation {
            ADD, SUB, DIV, MUL, MOD
        };

        ExpressionOperator(Json::Value &params);
        std::unique_ptr<TimeSeries> createTimeSeries(std::vector<TimeSeries *> &inputs,
                                                     std::shared_ptr<GenericOperator> op_ptr) override;
        Raster* executeOnRaster(Descriptor *descriptor) override;
    private:
        Operation getOperation();
        void setOperands(std::vector<TimeSeries *> &inputs);

        bool isATimeSeries;
        bool isBTimeSeries;
        double operandANum;
        double operandBNum;
        TimeSeries *operandATs;
        TimeSeries *operandBTs;
    };

}


#endif //RASTER_TIME_SERIES_EXPRESSION_H
