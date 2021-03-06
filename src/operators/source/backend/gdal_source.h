
#ifndef RASTER_TIME_SERIES_GDAL_SOURCE_H
#define RASTER_TIME_SERIES_GDAL_SOURCE_H

#include "operators/source/source_operator.h"
#include "util/gdal_util.h"
#include "util/time_interval.h"
#include <gdal_priv.h>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace rts {

    /**
     * Source operator loading rasters with the GDAL library. This supports any raster format supported by GDAL.
     * A GDALSource dataset is defined by a start and end point, and a time interval (time unit and value).
     * Therefore it represents rasters that are valid in regular time intervals.
     */
    class GDALSource : public SourceBackend {
    public:
        GDALSource(const QueryRectangle &qrect, const Json::Value &params);

        void initialize() override;
        bool supportsOrder(Order o) const override;
        OptionalDescriptor createDescriptor(double time, int pixelStartX, int pixelStartY, int tileIndex, const Resolution &rasterWorldPixelStart, const Scale &scale, const Origin &origin, const Resolution &tileCount) override;
        double getCurrentTimeEnd(double currTime) const override;
        void increaseCurrentTime(double &currTime) override;
        void beforeTemporalIncrease() override;
        Origin getOrigin() const override;
    private:
        std::shared_ptr<GDALDataset> currDataset;
        GDALRasterBand *currRasterband; //this can stay a normal ptr, because it is handled by the dataset. The dataset now always has to live as long as the rasterband. maybe put them in one structure?
        double currDatasetTime;

        TimeInterval timeInterval;
        std::string timeFormat;
        std::string baseFileName;
        std::string path;
        int channel;
        Origin origin;
        std::map<std::string, std::shared_ptr<GDALDataset>> openDatasets;

        Json::Value loadDatasetJson(const std::string &name);
        double parseIsoTime(const std::string &str) const;
        void loadCurrentGdalDataset(double time);
    };

}

#endif //RASTER_TIME_SERIES_GDAL_SOURCE_H
