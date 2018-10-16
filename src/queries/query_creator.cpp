
#include "operators/print.h"
#include "operators/expression.h"
#include "operators/source/fake_source.h"
#include "operators/sampler.h"
#include "operators/aggregator.h"
#include "queries/query_creator.h"

using namespace rts;

ConsumingOperator *QueryCreator::createOperatorTree(const Json::Value &query) {

    QueryRectangle qrect(query["query_rectangle"]);

    Json::Value sources_json = query["sources"];
    std::vector<GenericOperator*> sources = createSources(sources_json, qrect);

    Json::Value params = query["params"];
    return createConsumingOperator(query["operator"].asString(), qrect, params, sources);
}

std::vector<GenericOperator *> QueryCreator::createSources(Json::Value &sources, QueryRectangle &qrect) {

    if(!sources.isArray()){
        throw std::runtime_error("QueryCreator: sources must be a json array.");
    }

    std::vector<GenericOperator *> outList;

    for(Json::ArrayIndex i = 0; i < sources.size(); ++i){
        Json::Value &source = sources[i];
        std::vector<GenericOperator *> input_operators = createSources(source["sources"], qrect);
        outList.push_back(createOperator(source["operator"].asString(), qrect, source["params"], input_operators));
    }

    return outList;
}

ConsumingOperator *
QueryCreator::createConsumingOperator(const std::string &op_name, QueryRectangle qrect, Json::Value &params,
                                      const std::vector<GenericOperator *> &in)
{
    if(op_name == "print")
        return new Print(qrect, params, in);
    else
        throw std::runtime_error("Unknown operator: " + op_name);
}

GenericOperator *
QueryCreator::createOperator(const std::string &op_name, QueryRectangle qrect, Json::Value &params,
                             const std::vector<GenericOperator *> &in)
{
    if(op_name == "expression")
        return new Expression(qrect, params, in);
    else if(op_name == "fake_source")
        return new FakeSource(qrect, params);
    else if(op_name == "sampler")
        return new Sampler(qrect, params, in);
    else if(op_name == "aggregator")
        return new Aggregator(qrect, params, in);
    else
        throw std::runtime_error("Unknown operator: " + op_name);
}

GenericOperator *
QueryCreator::createOperator(const std::string &op_name, QueryRectangle qrect, Json::Value &params)
{
    if(op_name == "fake_source")
        return new FakeSource(qrect, params);
    else
        throw std::runtime_error("Unknown operator or needs input operators: " + op_name);
}
