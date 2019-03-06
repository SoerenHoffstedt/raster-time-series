
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
                               Resolution rasterTileCountDimensional,
                               double nodata,
                               GDALDataType dataType)
        : rasterInfo(totalInfo),
          tileSpatialInfo(tileSpatialInfo),
          tileResolution(tileResolution),
          order(order),
          tileIndex(tileIndex),
          rasterTileCountDimensional(rasterTileCountDimensional),
          rasterTileCount(rasterTileCountDimensional.resX * rasterTileCountDimensional.resY),
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
          rasterTileCountDimensional(desc->rasterTileCountDimensional),
          rasterTileCount(desc->rasterTileCount),
          nodata(desc->nodata),
          _isOnlyNodata(desc->_isOnlyNodata),
          dataType(desc->dataType)
{

}

DescriptorInfo &DescriptorInfo::operator=(const boost::optional<Descriptor> &desc) {
    rasterInfo = desc->rasterInfo;
    order = desc->order;
    tileSpatialInfo = desc->tileSpatialInfo;
    tileResolution = desc->tileResolution;
    tileIndex = desc->tileIndex;
    rasterTileCountDimensional = desc->rasterTileCountDimensional;
    rasterTileCount = desc->rasterTileCount;
    nodata = desc->nodata;
    _isOnlyNodata = desc->_isOnlyNodata;
    dataType = desc->dataType;
    return *this;
}

bool DescriptorInfo::isOnlyNodata() const {
    return _isOnlyNodata;
}

// Descriptor:

Descriptor::Descriptor(std::function<UniqueRaster(const Descriptor&)> &&getter,
                       const SpatialTemporalReference &totalInfo,
                       const SpatialReference &tileSpatialInfo,
                       const Resolution &tileResolution,
                       Order order,
                       uint32_t tileIndex,
                       Resolution rasterTileCountDimensional,
                       double nodata,
                       GDALDataType dataType)
        : getter(std::move(getter)),
          DescriptorInfo(totalInfo, tileSpatialInfo, tileResolution, order, tileIndex, rasterTileCountDimensional, nodata, dataType)
{

}

std::unique_ptr<Raster> Descriptor::getRaster() const {
    return getter(*this);
}

boost::optional<Descriptor> Descriptor::createNodataDescriptor(SpatialTemporalReference &totalInfo,
                                                             SpatialReference &tileSpatialInfo,
                                                             Resolution &tileResolution,
                                                             Order order,
                                                             uint32_t tileIndex,
                                                             Resolution rasterTileCountDimensional,
                                                             double nodata,
                                                             GDALDataType dataType)
{
    auto getter = [](const Descriptor &self) -> UniqueRaster {
        UniqueRaster raster = Raster::createRaster(self.dataType, self.tileResolution);
        //set all values to nodata with the AllValuesSetter
        RasterOperations::callUnary<RasterOperations::AllValuesSetter>(raster.get(), self.nodata);
        return raster;
    };
    auto ret = rts::make_optional<Descriptor>(std::move(getter), totalInfo, tileSpatialInfo, tileResolution, order, tileIndex, rasterTileCountDimensional, nodata, dataType);
    ret->_isOnlyNodata = true;
    return ret;
}

Descriptor::Descriptor(std::function<UniqueRaster(const Descriptor &)> &&getter, const DescriptorInfo &args)
        : getter(std::move(getter)), DescriptorInfo(args)
{

}
