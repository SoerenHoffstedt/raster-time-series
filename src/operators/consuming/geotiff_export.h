
#ifndef RASTER_TIME_SERIES_GEOTIFF_EXPORT_H
#define RASTER_TIME_SERIES_GEOTIFF_EXPORT_H

#include "operators/consuming/consuming_operator.h"

namespace rts {

    /**
     *
     */
    class GeotiffExport : public ConsumingOperator {
    public:
        explicit GeotiffExport(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        void consume() override;
        bool supportsOrder(Order o) override;
    private:
        std::string driverName;
        std::string path;
    };

}




#endif //RASTER_TIME_SERIES_GEOTIFF_EXPORT_H
