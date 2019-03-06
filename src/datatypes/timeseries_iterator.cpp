
#include "timeseries_iterator.h"

using namespace rts;

TimeSeriesIterator::TimeSeriesIterator(GenericOperator *op) : op(op) {
    next_descriptor = op == nullptr ? boost::none : op->nextDescriptor();
}

TimeSeriesIterator TimeSeriesIterator::createEndIterator() {
    return TimeSeriesIterator(nullptr);
}

Descriptor& TimeSeriesIterator::operator*() {
    return *next_descriptor;
}

Descriptor* TimeSeriesIterator::operator->() {
    return &(*next_descriptor);
}

TimeSeriesIterator &TimeSeriesIterator::operator++() {
    next_descriptor = op != nullptr ? op->nextDescriptor() : boost::none;
    return *this;
}

bool TimeSeriesIterator::operator==(const TimeSeriesIterator &other) const {
    if(!next_descriptor && !other.next_descriptor)
        return true;
    if(!next_descriptor || !other.next_descriptor)
        return false;
    if(other.op != op)
        return false;
    return next_descriptor->rasterInfo.t1 == other.next_descriptor->rasterInfo.t1 &&
           next_descriptor->rasterInfo.t2 == other.next_descriptor->rasterInfo.t2 &&
           next_descriptor->tileIndex == other.next_descriptor->tileIndex;

}

bool TimeSeriesIterator::operator!=(const TimeSeriesIterator &other) const {
    return !(*this == other);
}


