
#include <boost/filesystem.hpp>
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

    int countSuccessful = 0;
    int countFailed = 0;
    std::vector<std::string> failedQueries;

    for(auto &f : boost::filesystem::directory_iterator("../../test/query/")) {

        try {
            std::ifstream file_in(f.path().string());

            Json::Value json_query;
            file_in >> json_query;
            std::cout << "Query: " << f.path().filename() << std::endl;

            std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

            std::unique_ptr<OperatorTree> operatorTree = std::make_unique<OperatorTree>(json_query);
            std::unique_ptr<ConsumingOperator> p = operatorTree->instantiateConsuming();
            p->consume();

            std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

            std::cout << "\nQuery execution time: " << duration << " ms.\n" << std::endl;
            countSuccessful += 1;
        } catch (const std::exception &e){
            countFailed += 1;
            failedQueries.push_back(f.path().string());
            std::cout << "\nQuery failed: " << e.what() << std::endl;
        }
    }

    std::cout << "\nAll queries finished, successful: " << countSuccessful << ", failed: " << countFailed << ". Names of failed:" << std::endl;

    for(auto &name : failedQueries){
        std::cout << name << std::endl;
    }

    return 0;
}

