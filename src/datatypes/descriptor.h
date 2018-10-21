
#ifndef RASTER_TIME_SERIES_DESCRIPTOR_H
#define RASTER_TIME_SERIES_DESCRIPTOR_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "datatypes/raster.h"
#include "util/parameters.h"
#include "datatypes/spatial_temporal_reference.h"
#include "datatypes/order.h"

namespace rts {

    /**
     * Base class for descriptors, containing spatial temporal reference and virtual getRaster method.
     */
    class Descriptor {
    public:
        virtual ~Descriptor() = default;
        virtual std::unique_ptr<Raster> getRaster() = 0;
        QueryRectangle tileInfo;
        QueryRectangle totalInfo;
    protected:
        Descriptor(TemporalReference temp_ref_total, SpatialReference spat_ref_total, Resolution res_total, TemporalReference temp_ref_tile, SpatialReference spat_ref_tile, Resolution res_tile, Order order);
        Descriptor(QueryRectangle qrect_total, QueryRectangle qrect_tile);
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
        GenericDescriptor<GetterType>(GetterType getter, QueryRectangle qrect_total, QueryRectangle qrect_tile) : Descriptor(qrect_total, qrect_tile), getter(std::move(getter))
        {
            //TODO: check if qrect_total is smaller than qrect_tile (spatial or temporal) => error message that tile and total are mixed up?
        }
        GenericDescriptor<GetterType>(GetterType getter, TemporalReference temp_ref_total, SpatialReference spat_ref_total, Resolution res_total, TemporalReference temp_ref_tile, SpatialReference spat_ref_tile, Resolution res_tile, Order order)
                : Descriptor(temp_ref_total, spat_ref_total, res_total, temp_ref_tile, spat_ref_tile, res_tile, order), getter(std::move(getter))
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
