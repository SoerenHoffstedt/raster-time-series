
#include <iostream>
#include "datatypes/descriptor.h"
#include "descriptor.h"
#include <cmath>
#include "util/raster_calculations.h"
#include "datatypes/raster_operations.h"


using namespace rts;

// DescriptorInfo:

DescriptorInfo::DescriptorInfo(const SpatialTemporalReference &totalInfo,
                               const SpatialReference &tileSpatialInfo,
                               const Resolution &tileResolution,
                               Order order,
                               uint32_t tileIndex,
                               uint32_t tileCount,
                               double nodata,
                               GDALDataType dataType)
        : rasterInfo(totalInfo),
          tileSpatialInfo(tileSpatialInfo),
          tileResolution(tileResolution),
          order(order),
          tileIndex(tileIndex),
          rasterTileCount(tileCount),
          nodata(nodata),
          dataType(dataType)
{

}

DescriptorInfo::DescriptorInfo(const OptionalDescriptor &desc)
        : rasterInfo(desc->rasterInfo),
          order(desc->order),
          tileSpatialInfo(desc->tileSpatialInfo),
          tileResolution(desc->tileResolution),
          tileIndex(desc->tileIndex),
          rasterTileCount(desc->rasterTileCount),
          nodata(desc->nodata),
          _isOnlyNodata(desc->_isOnlyNodata),
          dataType(desc->dataType)
{

}

bool DescriptorInfo::isOnlyNodata() const {
    return _isOnlyNodata;
}

Resolution DescriptorInfo::getDimensionalTileCount() const {
    uint32_t num_x = rasterInfo.res_x / tileResolution.res_x;
    uint32_t num_y = rasterInfo.res_y / tileResolution.res_y;
    if(rasterInfo.res_x % tileResolution.res_x > 0)
        num_x += 1;
    if(rasterInfo.res_y % tileResolution.res_y > 0)
        num_y += 1;
    return Resolution(num_x, num_y);
}

// Descriptor:

Descriptor::Descriptor(std::function<UniqueRaster(const Descriptor&)> &&getter,
                       const SpatialTemporalReference &totalInfo,
                       const SpatialReference &tileSpatialInfo,
                       const Resolution &tileResolution,
                       Order order,
                       uint32_t tileIndex,
                       uint32_t tileCount,
                       double nodata,
                       GDALDataType dataType)
        : getter(std::move(getter)),
          DescriptorInfo(totalInfo, tileSpatialInfo, tileResolution, order, tileIndex, tileCount, nodata, dataType)
{

}

std::unique_ptr<Raster> Descriptor::getRaster() const {
    return getter(*this);
}

std::optional<Descriptor> Descriptor::createNodataDescriptor(SpatialTemporalReference &totalInfo,
                                                             SpatialReference &tileSpatialInfo,
                                                             Resolution &tileResolution,
                                                             Order order,
                                                             uint32_t tileIndex,
                                                             uint32_t tileCount,
                                                             double nodata,
                                                             GDALDataType dataType)
{
    auto getter = [](const Descriptor &self) -> UniqueRaster {
        UniqueRaster raster = Raster::createRaster(self.dataType, self.tileResolution);
        //set all values to nodata with the AllValuesSetter
        RasterOperations::callUnary<RasterOperations::AllValuesSetter>(raster.get(), self.nodata);
        return raster;
    };
    auto ret = std::make_optional<Descriptor>(std::move(getter), totalInfo, tileSpatialInfo, tileResolution, order, tileIndex, tileCount, nodata, dataType);
    ret->_isOnlyNodata = true;
    return ret;
}

Descriptor::Descriptor(std::function<UniqueRaster(const Descriptor &)> &&getter, const DescriptorInfo &args)
        : getter(std::move(getter)), DescriptorInfo(args)
{

}
