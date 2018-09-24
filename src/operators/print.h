
#ifndef RASTER_TIME_SERIES_PRINT_H
#define RASTER_TIME_SERIES_PRINT_H

#include "operators/generic_operator.h"

namespace rts {

    /**
     * Operator printing the data of the input rasters to std::cout. For testing small data.
     */
    class Print : public GenericOperator {
    public:
        explicit Print(Json::Value &params, std::vector<GenericOperator*> in);
        UniqueDescriptor next() override;
    };

}

#endif //RASTER_TIME_SERIES_PRINT_H
