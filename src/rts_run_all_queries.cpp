
#include <filesystem>
#include <chrono>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include "queries/query_creator.h"
#include "operators/consuming/print.h"
#include "operators/expression_operator.h"
#include "operators/source/fake_source.h"


int main(int argc, char** argv) {

    using namespace rts;

    int count = 0;

    for(auto &f : std::filesystem::directory_iterator("../../test/query/")) {

        std::ifstream file_in(f.path().string());

        Json::Value json_query;
        file_in >> json_query;
        std::cout << "Query: " << f.path().filename() << std::endl;

        QueryCreator queryCreator;

        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

        std::unique_ptr<ConsumingOperator> p = queryCreator.createOperatorTree(json_query);
        p->consume();

        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        std::cout << "\nQuery execution time: " << duration << " ms." << std::endl;
        ++count;
    }

    std::cout << "All queries finished, count: " << count << std::endl;

    return 0;
}

