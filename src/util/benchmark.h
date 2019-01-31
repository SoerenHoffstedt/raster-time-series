
#ifndef RASTER_TIME_SERIES_BENCHMARK_H
#define RASTER_TIME_SERIES_BENCHMARK_H

#include <fstream>
#include <chrono>

namespace rts {

    using std::chrono;

    class Benchmark {
    public:
        static void startQuery(std::ifstream &fileOutput);
        static void endQuery();
        static void startSource();
        static void endSource();
        static void startConsuming();
        static void endConsuming();
    private:
        high_resolution_clock queryStart;
        high_resolution_clock sourceStart;
        high_resolution_clock consumingStart;
        duration<milliseconds> sourceDuration;
        duration<milliseconds> consumingDuration;

    };

}

#endif //RASTER_TIME_SERIES_BENCHMARK_H
