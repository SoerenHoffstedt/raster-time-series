
#ifndef RASTER_TIME_SERIES_GDAL_SOURCE_H
#define RASTER_TIME_SERIES_GDAL_SOURCE_H

#include "operators/generic_operator.h"
#include "util/gdal_util.h"
#include "util/time_interval.h"
#include <gdal_priv.h>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace rts {

    class GDALSource : public GenericOperator {
    public:
        GDALSource(const QueryRectangle &qrect, Json::Value &params,
                   std::vector<std::unique_ptr<GenericOperator>> &&in);
        OptionalDescriptor nextDescriptor() override;
        bool supportsOrder(Order o) const override;
    private:
        std::shared_ptr<GDALDataset> currDataset;
        GDALRasterBand *currRasterband; //this can stay a normal ptr, because it is handled by the dataset. The dataset now always has to live as long as the rasterband. maybe put them in one structure?

        Resolution tileRes;
        int currRasterIndex;
        int currTileIndex;
        int state_x;
        int state_y;
        Resolution rasterWorldPixelStart;
        uint32_t tileCount;

        boost::posix_time::ptime curr_time;
        boost::posix_time::ptime dataset_time_start;
        boost::posix_time::ptime dataset_time_end;

        TimeInterval time_interval;
        std::string time_format;
        std::string file_name_base;
        std::string path;
        int channel;

        Json::Value loadDatasetJson(const std::string &name);
        double parseIsoTime(const std::string &str) const;
        bool increaseSpatial();
        bool increaseTemporal();
        void loadCurrentGdalDataset();
        void increaseCurrentTime();
        double getCurrentTimeEnd() const;
    };

}

#endif //RASTER_TIME_SERIES_GDAL_SOURCE_H
