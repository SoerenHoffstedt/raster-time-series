
#include "benchmark.h"

using namespace rts;
using namespace std::chrono;

//init static members
std::ofstream* Benchmark::outputFile = nullptr;
high_resolution_clock::time_point Benchmark::queryStart;
high_resolution_clock::time_point Benchmark::sourceStart;
high_resolution_clock::time_point Benchmark::consumingStart;
milliseconds Benchmark::sourceDuration(0);
milliseconds Benchmark::consumingDuration(0);

void Benchmark::setFileOutputStream(std::ofstream &outputFile) {
    Benchmark::outputFile = &outputFile;
}

void Benchmark::endQuery() {
    auto totalQuery = duration_cast<milliseconds>(high_resolution_clock::now() - queryStart).count();
    (*outputFile) << sourceDuration.count() << std::endl;
    (*outputFile) << consumingDuration.count() << std::endl;
    (*outputFile) << totalQuery << std::endl;
}

void Benchmark::startSource() {
    sourceStart = std::chrono::high_resolution_clock::now();
}

void Benchmark::startQuery() {
    queryStart = std::chrono::high_resolution_clock::now();
    sourceDuration = milliseconds(0);
    consumingDuration = milliseconds(0);
}

void Benchmark::endSource() {
    auto now = high_resolution_clock::now();
    sourceDuration += duration_cast<milliseconds>(now - sourceStart);
}

void Benchmark::startConsuming() {
    consumingStart = std::chrono::high_resolution_clock::now();
}

void Benchmark::endConsuming() {
    auto now = high_resolution_clock::now();
    consumingDuration += duration_cast<milliseconds>( now - consumingStart );
}



