
#ifndef RASTER_TIME_SERIES_PRINT_H
#define RASTER_TIME_SERIES_PRINT_H

#include "operators/generic_operator.h"
#include "operators/consuming/consuming_operator.h"

namespace rts {

    /**
     * Operator printing the data of the input rasters to std::cout. For testing small data.
     */
    class Print : public ConsumingOperator {
    public:
        explicit Print(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        void consume() override;
        bool supportsOrder(Order o) const override;
    };

}

#endif //RASTER_TIME_SERIES_PRINT_H
