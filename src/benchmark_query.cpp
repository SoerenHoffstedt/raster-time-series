
#include <chrono>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include <util/benchmark.h>
#include "queries/operator_tree.h"
#include "operators/consuming/print.h"
#include "operators/expression_operator.h"
#include "operators/source/fake_source.h"


/**
 * Executes a query multiple times, tracks execution time for each query, and prints it into a file.
 * Takes three input parameters: query file name, output file name, number of repeated execution.
 * The output file contains three numbers for each query execution: source duration, consuming duration,
 * and total query duration. Every number is written in a single liner and all numbers are in milliseconds.
 * The three numbers of every query are simply written underneath each other without
 * any differentiation except their line number.
 */
int main(int argc, char** argv) {

    using namespace rts;

    if(argc < 2) {
        std::cout << "No query file provided in program arguments." << std::endl;
        return 0;
    }

    int count = std::stoi(argv[3]);

    std::string outputFilename = argv[2];
    std::ofstream outputFile(outputFilename);

    Benchmark::setFileOutputStream(outputFile);

    for(int i = 0; i < count; ++i){
        std::string queryFilename(argv[1]);
        std::ifstream file_in(queryFilename);

        Json::Value json_query;
        file_in >> json_query;

        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

        Benchmark::startQuery();

        std::unique_ptr<OperatorTree> operatorTree = std::make_unique<OperatorTree>(json_query);
        std::unique_ptr<ConsumingOperator> p = operatorTree->instantiateConsuming();
        p->consume();

        Benchmark::endQuery();

    }
    outputFile.close();

    return 0;
}
