
#ifndef RASTER_TIME_SERIES_AGGREGATOR_H
#define RASTER_TIME_SERIES_AGGREGATOR_H

#include "generic_operator.h"

namespace rts {

    /***
     * Parameters:
     *  - custom_data_type: [Byte, UInt16, Int16, UInt32, Int32, Float32, Float64], when not provided data type of input tiles is used.
     *
     *
     */
    class Aggregator : public GenericOperator {
    public:
        Aggregator(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        OptionalDescriptor nextDescriptor() override;
        bool supportsOrder(Order order) override;
    private:
        OptionalDescriptor nextDesc;
        GDALDataType customDataType;
    };

}


#endif //RASTER_TIME_SERIES_AGGREGATOR_H
