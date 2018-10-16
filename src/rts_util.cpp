
#include <iostream>
#include <fstream>
#include <json/json.h>
#include "queries/query_creator.h"
#include "operators/print.h"
#include "operators/expression.h"
#include "operators/source/fake_source.h"


int main() {

    using namespace rts;

    std::ifstream file_in("../../test/query/test_query.json");

    Json::Value json_query;
    file_in >> json_query;

    QueryCreator queryCreator;

    ConsumingOperator *p = queryCreator.createOperatorTree(json_query);
    p->consume();
    delete p;
    
    return 0;
}
