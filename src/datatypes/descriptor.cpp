
#include <iostream>
#include "datatypes/descriptor.h"
#include "descriptor.h"
#include <cmath>
#include "util/raster_calculations.h"


using namespace rts;

Descriptor::Descriptor(std::function<UniqueRaster(const Descriptor&)> &&getter,
                       const SpatialTemporalReference &totalInfo,
                       const SpatialReference &tileSpatialInfo,
                       const Resolution &tileResolution,
                       Order order,
                       uint32_t tileIndex,
                       uint32_t tileCount,
                       int nodata)
        : getter(std::move(getter)),
          rasterInfo(totalInfo),
          order(order),
          tileSpatialInfo(tileSpatialInfo),
          tileResolution(tileResolution),
          tileIndex(tileIndex),
          rasterTileCount(tileCount),
          nodata(nodata),
          _isOnlyNodata(false) { }

std::unique_ptr<Raster> Descriptor::getRaster() const {
    return getter(*this);
}

bool Descriptor::isOnlyNodata() const {
    return _isOnlyNodata;
}

std::optional<Descriptor> Descriptor::createNodataDescriptor(SpatialTemporalReference &totalInfo,
                                                             SpatialReference &tileSpatialInfo,
                                                             Resolution &tileResolution,
                                                             Order order,
                                                             uint32_t tileIndex,
                                                             uint32_t tileCount,
                                                             int nodata)
{
    auto getter = [tileResolution = tileResolution](const Descriptor &self) -> UniqueRaster {
        UniqueRaster raster = std::make_unique<Raster>(tileResolution);
        for (int x = 0; x < tileResolution.res_x; ++x) {
            for (int y = 0; y < tileResolution.res_y; ++y) {
                raster->setCell(x, y, self.nodata);
            }
        }
        return raster;
    };
    auto ret = std::make_optional<Descriptor>(std::move(getter), totalInfo, tileSpatialInfo, tileResolution, order, tileIndex, tileCount, nodata);
    ret->_isOnlyNodata = true;
    return ret;
}
