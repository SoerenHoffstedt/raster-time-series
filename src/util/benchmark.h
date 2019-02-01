
#ifndef RASTER_TIME_SERIES_BENCHMARK_H
#define RASTER_TIME_SERIES_BENCHMARK_H

#include <fstream>
#include <chrono>

namespace rts {

    /**
     * Utility class for benchmarking execution time of queries.
     * Usage: set output file stream.
     * Call startQuery() before start of the execution of the query.
     * After execution of the query is finished, call endQuery().
     * In the consuming operator call startConsuming() when entering and endConsuming() when exiting the consuming operator.
     * The same applies for the source operator and start/endSource().
     * endQuery() will write three lines into the output file. First line is the milliseconds spend in the source
     * operator, second line is ms spend in consuming operator, and the third line is the total time spend in the query.
     * The last number includes the time spend in source and consuming operator.
     */
    class Benchmark {
    public:
        static void setFileOutputStream(std::ofstream &outputFile);
        static void startQuery();
        static void endQuery();
        static void startSource();
        static void endSource();
        static void startConsuming();
        static void endConsuming();
    private:
        static std::ofstream *outputFile;
        static std::chrono::high_resolution_clock::time_point queryStart;
        static std::chrono::high_resolution_clock::time_point sourceStart;
        static std::chrono::high_resolution_clock::time_point consumingStart;
        static std::chrono::milliseconds sourceDuration;
        static std::chrono::milliseconds consumingDuration;

    };

}

#endif //RASTER_TIME_SERIES_BENCHMARK_H
