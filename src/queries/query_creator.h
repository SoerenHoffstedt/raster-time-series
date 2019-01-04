
#ifndef RASTER_TIME_SERIES_QUERY_CREATOR_H
#define RASTER_TIME_SERIES_QUERY_CREATOR_H

#include "operators/consuming/consuming_operator.h"

namespace rts {

    class QueryCreator {
    public:
        std::unique_ptr<ConsumingOperator> createOperatorTree(const Json::Value &query);
    private:
        std::unique_ptr<ConsumingOperator> createConsumingOperator(const std::string &op_name, QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        std::unique_ptr<GenericOperator> createOperator(const std::string &op_name, QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);

        std::vector<std::unique_ptr<GenericOperator>> createSources(Json::Value &sources, QueryRectangle &qrect);
        void callInitialize(GenericOperator *op);
    };

}

#endif //RASTER_TIME_SERIES_QUERY_CREATOR_H
