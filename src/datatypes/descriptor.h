
#ifndef RASTER_TIME_SERIES_DESCRIPTOR_H
#define RASTER_TIME_SERIES_DESCRIPTOR_H

#include <string>
#include <vector>
#include <memory>
#include "datatypes/raster.h"
#include "operators/generic_operator.h"
#include "util/parameters.h"

namespace rts {

    class GenericOperator;

    class Descriptor {
    public:
        Descriptor(int id, int res_x, int res_y, double time_start, double time_duration, std::vector<Descriptor*> inputs, std::shared_ptr<GenericOperator> operator_ptr);
        Descriptor(const Descriptor &other) = default;
        Descriptor(Descriptor &&other) = default;
        ~Descriptor();

        int id;
        double time_start;
        double time_duration;
        int min;
        int max;
        int res_x;
        int res_y;

        Raster* getRasterPtr();
        bool isRasterLoaded() const;
        unsigned long inputCount() const {
            return inputs.size();
        }
        Descriptor* getInput(int i){
            return inputs[i];
        }
        int getId() const {
            return id;
        }
        bool isValidBetween(double low, double high) const;

    private:
        Raster* raster_ptr;
        std::shared_ptr<GenericOperator> operator_ptr;
        std::vector<Descriptor*> inputs;
    };

}

#endif //RASTER_TIME_SERIES_DESCRIPTOR_H
