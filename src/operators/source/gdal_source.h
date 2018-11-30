
#ifndef RASTER_TIME_SERIES_GDAL_SOURCE_H
#define RASTER_TIME_SERIES_GDAL_SOURCE_H

#include "operators/generic_operator.h"
#include "util/gdal_util.h"
#include <gdal_priv.h>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace rts {

    class GDALSource : public GenericOperator {
    public:
        GDALSource(const QueryRectangle &qrect, Json::Value &params,
                   std::vector<std::unique_ptr<GenericOperator>> &&in);
        OptionalDescriptor nextDescriptor() override;
        bool supportsOrder(Order o) override;
    private:
        GDALDataset *currDataset;
        GDALRasterBand *currRasterband;

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

        TimeUnit time_interval_unit;
        uint32_t time_interval_value;
        std::string time_format;
        std::string file_name_base;
        std::string path;
        int channel;

        Json::Value loadDatasetJson(const std::string &name);
        TimeUnit parseTimeIntervalUnit(const std::string &str);
        double parseIsoTime(const std::string &str) const;
        bool increaseSpatial();
        bool increaseTemporal();
        void loadCurrentGdalDataset();
        void increaseCurrentTime();
        double getCurrentTimeEnd() const;
        boost::posix_time::ptime increasePTimeByInterval(boost::posix_time::ptime to_increase) const;
    };

}

#endif //RASTER_TIME_SERIES_GDAL_SOURCE_H
