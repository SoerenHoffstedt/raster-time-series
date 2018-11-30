
#include <iostream>
#include <fstream>
#include <json/json.h>
#include "queries/query_creator.h"
#include "operators/consuming/print.h"
#include "operators/expression.h"
#include "operators/source/fake_source.h"


int main(int argc, char** argv) {

    using namespace rts;

    if(argc < 2) {
        std::cout << "no query file provided in program arguments." << std::endl;
        return 0;
    }

    std::ifstream file_in(argv[1]);

    Json::Value json_query;
    file_in >> json_query;

    QueryCreator queryCreator;

    std::unique_ptr<ConsumingOperator> p = queryCreator.createOperatorTree(json_query);
    p->consume();

    return 0;
}
