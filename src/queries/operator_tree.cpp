
#include "operator_tree.h"
#include "operators/temporal_overlap.h"
#include "operators/cumulative_sum.h"
#include "operators/consuming/print.h"
#include "operators/consuming/geotiff_export.h"
#include "operators/consuming/raster_value_extraction.h"
#include "operators/expression_operator.h"
#include "operators/source/fake_source.h"
#include "operators/source/gdal_source.h"
#include "operators/sampler.h"
#include "operators/aggregator.h"
#include "operators/convolution.h"
#include "operators/order_changer.h"
#include "operators/cache_simulator.h"

using namespace rts;

OperatorTree::OperatorTree(const Json::Value &query)
        : operator_name(query["operator"].asString()), params(query["params"]), qrect(query["query_rectangle"]), isConsuming(true)
{
    createChildren(query["sources"]);
}

OperatorTree::OperatorTree(const Json::Value &query, QueryRectangle &qrect)
        : operator_name(query["operator"].asString()), params(query["params"]), qrect(qrect), isConsuming(false)
{
    createChildren(query["sources"]);
}

void OperatorTree::createChildren(const Json::Value &sourcesJson){
    for(int i = 0; i < sourcesJson.size(); ++i){
        auto &source = sourcesJson[i];
        children.push_back(new OperatorTree(source, qrect));
    }
}

OperatorTree::~OperatorTree() {
    for(auto *child : children){
        delete child;
    }
}

std::unique_ptr<GenericOperator> OperatorTree::instantiate() const {

    UniqueOperatorVector sources;
    for(auto *child : children){
        sources.emplace_back(child->instantiate());
    }

    std::unique_ptr<GenericOperator> res;

    if(operator_name == "expression")
        res = std::make_unique<ExpressionOperator>(this, qrect, params, std::move(sources));
    else if(operator_name == "fake_source")
        res = std::make_unique<FakeSource>(this, qrect, params, std::move(sources));
    else if(operator_name == "gdal_source")
        res = std::make_unique<GDALSource>(this, qrect, params, std::move(sources));
    else if(operator_name == "sampler")
        res = std::make_unique<Sampler>(this, qrect, params, std::move(sources));
    else if(operator_name == "aggregator")
        res = std::make_unique<Aggregator>(this, qrect, params, std::move(sources));
    else if(operator_name == "cumulative_sum")
        res = std::make_unique<CumulativeSum>(this, qrect, params, std::move(sources));
    else if(operator_name == "temporal_overlap")
        res = std::make_unique<TemporalOverlap>(this, qrect, params, std::move(sources));
    else if(operator_name == "convolution")
        res = std::make_unique<Convolution>(this, qrect, params, std::move(sources));
    else if(operator_name == "order_changer")
        res = std::make_unique<OrderChanger>(this, qrect, params, std::move(sources));
    else if(operator_name == "cache_simulator")
        res = std::make_unique<CacheSimulator>(this, qrect, params, std::move(sources));
    else
        throw std::runtime_error("Unknown operator: " + operator_name);

    return res;
}

std::unique_ptr<ConsumingOperator> OperatorTree::instantiateConsuming() const {

    if(!isConsuming)
        throw std::runtime_error("instantiateConsuming called on an OperatorTree that is not a consuming operator.");

    UniqueOperatorVector sources;
    for(auto *child : children){
        sources.emplace_back(child->instantiate());
    }

    std::unique_ptr<ConsumingOperator> res;

    if(operator_name == "print")
        res = std::make_unique<Print>(this, qrect, params, std::move(sources));
    else if(operator_name == "geotiff_export")
        res = std::make_unique<GeotiffExport>(this, qrect, params, std::move(sources));
    else if(operator_name == "raster_value_extraction")
        res =  std::make_unique<RasterValueExtraction>(this, qrect, params, std::move(sources));
    else
        throw std::runtime_error("Unknown operator: " + operator_name);

    res->initializeRecursively();
    return res;
}
