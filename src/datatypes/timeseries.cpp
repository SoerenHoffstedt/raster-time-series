
#include "timeseries.h"

using namespace rts;

rts::TimeSeries::iterator rts::TimeSeries::begin(){
    return descriptors.begin();
}

rts::TimeSeries::iterator rts::TimeSeries::end() {
    return descriptors.end();
}

void rts::TimeSeries::add(rts::Descriptor descriptor)
{
    descriptors.push_back(descriptor);
}

void rts::TimeSeries::move(rts::Descriptor &&descriptor) {
    descriptors.emplace_back(std::move(descriptor));
}

std::unique_ptr<TimeSeries> rts::TimeSeries::between(double time_start, double time_end) {
    UniquePtrTimeSeries ts = make_unique<TimeSeries>();

    for(const Descriptor &d : *this){
        if(d.isValidBetween(time_start, time_end)){
            Descriptor desc_new(d);
            ts->move(std::move(desc_new));
        }
    }

    return ts;
}

std::unique_ptr<TimeSeries> rts::TimeSeries::atTime(double time) {
    return between(time, time);
}

std::unique_ptr<TimeSeries> TimeSeries::sample(unsigned int sample_count) {

    if(sample_count > descriptors.size())
        throw std::runtime_error("Too many samples are requested, time series does not have that many rasters.");

    UniquePtrTimeSeries ts = make_unique<TimeSeries>();

    unsigned long steps = descriptors.size() / sample_count;
    int index = 0;
    for(int i = 0; i < sample_count; i++){
        ts->add(descriptors[index]);
        index += steps;
    }

    return ts;
}
