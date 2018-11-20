
#ifndef RASTER_TIME_SERIES_SPATIAL_OVERLAP_H
#define RASTER_TIME_SERIES_SPATIAL_OVERLAP_H

#include "operators/generic_operator.h"

namespace rts {

    class SpatialOverlap : public GenericOperator {
    public:
        SpatialOverlap(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        OptionalDescriptor nextDescriptor() override;
        bool supportsOrder(Order order) override;
    };

}

#endif //RASTER_TIME_SERIES_SPATIAL_OVERLAP_H
