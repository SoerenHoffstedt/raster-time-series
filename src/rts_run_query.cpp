
#include <chrono>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include "queries/operator_tree.h"
#include "operators/consuming/print.h"
#include "operators/expression_operator.h"
#include "operators/source/fake_source.h"

int main(int argc, char** argv) {

    using namespace rts;

    if(argc < 2) {
        std::cout << "no query file provided in program arguments." << std::endl;
        return 0;
    }

    std::string filename("../../test/query/");
    filename += argv[1];
    std::ifstream file_in(filename);

    Json::Value json_query;
    file_in >> json_query;

    std::cout << "Query: " << argv[1] << std::endl;

    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    std::unique_ptr<OperatorTree> operatorTree = std::make_unique<OperatorTree>(json_query);
    std::unique_ptr<ConsumingOperator> p = operatorTree->instantiateConsuming();
    p->consume();

    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1).count();

    std::cout << "\nQuery execution time: " << duration << " ms." << std::endl;

    return 0;
}
