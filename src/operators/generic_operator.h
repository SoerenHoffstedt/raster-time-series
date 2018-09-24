
#ifndef RASTER_TIME_SERIES_GENERIC_OPERATOR_H
#define RASTER_TIME_SERIES_GENERIC_OPERATOR_H

#include <json/json.h>
#include <boost/optional.hpp>
#include <vector>
#include "datatypes/raster.h"
#include "util/parameters.h"
#include "util/make_unique.h"
#include "datatypes/descriptor.h"

namespace rts {

    class Descriptor;

    class GenericOperator {
    public:
        explicit GenericOperator(Json::Value &params, const std::vector<GenericOperator*> &in);
        explicit GenericOperator(Json::Value &params);
        virtual ~GenericOperator() = default;
        virtual UniqueDescriptor next() = 0;
        void in(GenericOperator* in);

    protected:
        Json::Value params;
        std::vector<GenericOperator*> input_operators;
        /**
         * Throws an exception if the size of input_operators differs from expected.
         * @param expected the amount of expected input operators.
         */
        void checkInputCount(int expected);
    };

}


#endif //RASTER_TIME_SERIES_GENERIC_OPERATOR_H
