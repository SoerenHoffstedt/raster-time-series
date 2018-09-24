
#ifndef RASTER_TIME_SERIES_TIMESERIES_ITERATOR_H
#define RASTER_TIME_SERIES_TIMESERIES_ITERATOR_H

#include <iterator>

namespace rts {

    template <class DATA_TYPE>
    class TimeSeriesIterator : std::iterator<std::random_access_iterator_tag,
                                             DATA_TYPE,
                                             std::ptrdiff_t,
                                             DATA_TYPE*,
                                             DATA_TYPE&>
    {
    public:

    private:

    };

}



#endif //RASTER_TIME_SERIES_TIMESERIES_ITERATOR_H
