
#include "operators/temporal_overlap.h"
#include "operators/cumulative_sum.h"
#include "operators/consuming/print.h"
#include "operators/consuming/geotiff_export.h"
#include "operators/expression_operator.h"
#include "operators/source/fake_source.h"
#include "operators/source/gdal_source.h"
#include "operators/sampler.h"
#include "operators/aggregator.h"
#include "queries/query_creator.h"

using namespace rts;

std::unique_ptr<ConsumingOperator> QueryCreator::createOperatorTree(const Json::Value &query) {

    QueryRectangle qrect(query["query_rectangle"]);

    Json::Value sources_json = query["sources"];
    std::vector<std::unique_ptr<GenericOperator>> sources = createSources(sources_json, qrect);

    Json::Value params = query["params"];
    return createConsumingOperator(query["operator"].asString(), qrect, params, std::move(sources));
}

std::vector<std::unique_ptr<GenericOperator>> QueryCreator::createSources(Json::Value &sources, QueryRectangle &qrect) {

    if(!sources.isArray()){
        throw std::runtime_error("QueryCreator: sources must be a json array.");
    }

    std::vector<std::unique_ptr<GenericOperator>> outList;

    for(Json::ArrayIndex i = 0; i < sources.size(); ++i){
        Json::Value &source = sources[i];
        outList.push_back(
                createOperator(source["operator"].asString(),
                qrect,
                source["params"],
                createSources(source["sources"], qrect))
        );
    }

    return outList;
}

std::unique_ptr<ConsumingOperator>
QueryCreator::createConsumingOperator(const std::string &op_name, QueryRectangle qrect, Json::Value &params,
                                      std::vector<std::unique_ptr<GenericOperator>> &&in)
{
    if(op_name == "print")
        return std::make_unique<Print>(qrect, params, std::move(in));
    else if(op_name == "geotiff_export")
        return std::make_unique<GeotiffExport>(qrect, params, std::move(in));
    else
        throw std::runtime_error("Unknown operator: " + op_name);
}

std::unique_ptr<GenericOperator>
QueryCreator::createOperator(const std::string &op_name, QueryRectangle qrect, Json::Value &params,
                             std::vector<std::unique_ptr<GenericOperator>> &&in)
{
    if(op_name == "expression")
        return std::make_unique<ExpressionOperator>(qrect, params, std::move(in));
    else if(op_name == "fake_source")
        return std::make_unique<FakeSource>(qrect, params, std::move(in));
    else if(op_name == "gdal_source")
        return std::make_unique<GDALSource>(qrect, params, std::move(in));
    else if(op_name == "sampler")
        return std::make_unique<Sampler>(qrect, params, std::move(in));
    else if(op_name == "aggregator")
        return std::make_unique<Aggregator>(qrect, params, std::move(in));
    else if(op_name == "cumulative_sum")
        return std::make_unique<CumulativeSum>(qrect, params, std::move(in));
    else if(op_name == "temporal_overlap")
        return std::make_unique<TemporalOverlap>(qrect, params, std::move(in));
    else
        throw std::runtime_error("Unknown operator: " + op_name);
}
