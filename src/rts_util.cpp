
#include <chrono>
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

    std::cout << "Query: " << argv[1] << std::endl;

    QueryCreator queryCreator;

    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    std::unique_ptr<ConsumingOperator> p = queryCreator.createOperatorTree(json_query);
    p->consume();

    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1).count();

    std::cout << "\nQuery execution time: " << duration << " ms." << std::endl;


    return 0;
}
