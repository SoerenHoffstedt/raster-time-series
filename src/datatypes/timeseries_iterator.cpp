
#include "timeseries_iterator.h"

using namespace rts;

TimeSeriesIterator::TimeSeriesIterator(GenericOperator *op) : op(op) {
    next_descriptor = op == nullptr ? nullptr : op->next();
}

TimeSeriesIterator TimeSeriesIterator::createEndIterator() {
    return TimeSeriesIterator(nullptr);
}

Descriptor& TimeSeriesIterator::operator*() const {
    return *next_descriptor.get();
}

Descriptor* TimeSeriesIterator::operator->() const {
    return next_descriptor.get();
}

TimeSeriesIterator &TimeSeriesIterator::operator++() {
    next_descriptor = op != nullptr ? op->next() : nullptr;
    return *this;
}

bool TimeSeriesIterator::operator==(const TimeSeriesIterator &other) const {
    //TODO: how to test equality? There can not be two Iterators with same next_descriptor because it is unique_ptr.
    // For now only test if both are nullptr Iterators
    return next_descriptor == nullptr && other.next_descriptor == nullptr;
}

bool TimeSeriesIterator::operator!=(const TimeSeriesIterator &other) const {
    return !(*this == other);
}


