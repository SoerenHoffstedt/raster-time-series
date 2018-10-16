
#ifndef RASTER_TIME_SERIES_QUERY_CREATOR_H
#define RASTER_TIME_SERIES_QUERY_CREATOR_H

#include "operators/consuming_operator.h"

namespace rts {

    class QueryCreator {
    public:
        ConsumingOperator *createOperatorTree(const Json::Value &query);
    private:
        ConsumingOperator *createConsumingOperator(const std::string &op_name, QueryRectangle qrect, Json::Value &params, const std::vector<GenericOperator*> &in);
        GenericOperator *createOperator(const std::string &op_name, QueryRectangle qrect, Json::Value &params, const std::vector<GenericOperator*> &in);
        ConsumingOperator *createConsumingOperator(const std::string &op_name, QueryRectangle qrect, Json::Value &params);
        GenericOperator *createOperator(const std::string &op_name, QueryRectangle qrect, Json::Value &params);

        std::vector<GenericOperator *> createSources(Json::Value &sources, QueryRectangle &qrect);
    };

}

#endif //RASTER_TIME_SERIES_QUERY_CREATOR_H
