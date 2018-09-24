
#ifndef RASTER_TIME_SERIES_DESCRIPTOR_H
#define RASTER_TIME_SERIES_DESCRIPTOR_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "datatypes/raster.h"
#include "util/parameters.h"
#include "datatypes/spatial_temporal_reference.h"

namespace rts {

    /**
     * Base class for descriptors, containing spatial temporal reference and virtual getRaster method.
     */
    class Descriptor {
    public:
        virtual ~Descriptor() = default;
        SpatialTemporalReference st_ref;
        virtual std::unique_ptr<Raster> getRaster() = 0;
    protected:
        Descriptor(double time_start, double time_end, double x1, double x2, double y1, double y2, uint32_t res_x, uint32_t res_y);
        Descriptor(TemporalReference temp_ref, SpatialReference spat_ref, Resolution res);
        Descriptor(SpatialTemporalReference st_ref);
    };

    /**
     * Each closure for getting the raster, created by the operators, has a different anonymous type. To avoid using
     * std::function as a member the getter closure is a generic template parameter.
     * @tparam GetterType The type of the closure that's called in getRaster. It needs to be callable with operator()
     * returning a std::unique_ptr<Raster>.
     */
    template <class GetterType>
    class GenericDescriptor : public Descriptor {
    public:
        GenericDescriptor<GetterType>(GetterType getter, double time_start, double time_end, double x1, double x2, double y1, double y2, uint32_t res_x, uint32_t  res_y)
                : Descriptor(time_start, time_end, x1, x2, y1, y2, res_x, res_y), getter(std::move(getter))
        {

        }
        GenericDescriptor<GetterType>(GetterType getter, SpatialTemporalReference st_ref) : Descriptor(st_ref), getter(std::move(getter))
        {

        }
        GenericDescriptor<GetterType>(GetterType getter, TemporalReference temp_ref, SpatialReference spat_ref, Resolution res)
                : Descriptor(temp_ref, spat_ref, res), getter(std::move(getter))
        {

        }

        UniqueRaster getRaster() override {
            return getter(this);
        }
    private:
        GetterType getter;
    };

    using UniqueDescriptor = std::unique_ptr<Descriptor>;

    /**
     * Template function for returning a unique_ptr to a Descriptor. The type of the GenericDescriptor depends on the
     * passed getter. In combination with the getter the args have to form a valid constructor of GenericDescriptor.
     */
    template <class GetterType, class ...Args>
    static UniqueDescriptor createUniqueDescriptor(GetterType &&getter, Args&&... args){
        return std::make_unique<GenericDescriptor<GetterType>>(std::forward<GetterType>(getter), std::forward<Args>(args)...);
    }

}

#endif //RASTER_TIME_SERIES_DESCRIPTOR_H
