
#ifndef RASTER_TIME_SERIES_TIMESERIES_ITERATOR_H
#define RASTER_TIME_SERIES_TIMESERIES_ITERATOR_H

#include <iterator>
#include "datatypes/descriptor.h"
#include "operators/generic_operator.h"

namespace rts {

    class GenericOperator;

    /**
     * Forward iterator for the raster Descriptors of an operator. Could be expanded to a bidirectional iterator
     * if operators provide previous() function analogous to next(). Random access could be achieved by adding
     * a state counter to each operator.
     *
     * Lifetime of a iterator should not exceed the lifetime of the operator that created it.
     * The lifetime of the returned references/pointers is bound to the lifetime of the iterator object because
     * it safes the Descriptor as unique_ptr.
     */
    class TimeSeriesIterator : std::iterator<std::forward_iterator_tag,
                                             Descriptor,
                                             std::ptrdiff_t,
                                             Descriptor*,
                                             Descriptor&>
    {
    public:
        TimeSeriesIterator(GenericOperator *op);
        static TimeSeriesIterator createEndIterator();
        // Get the data element at this position
        Descriptor& operator*() const;
        Descriptor* operator->() const;

        // get next element
        TimeSeriesIterator& operator++();

        // Comparison operators
        bool operator== (const TimeSeriesIterator &other) const;
        bool operator!= (const TimeSeriesIterator &other) const;

    private:
        GenericOperator *op;
        UniqueDescriptor next_descriptor;
    };

}



#endif //RASTER_TIME_SERIES_TIMESERIES_ITERATOR_H
