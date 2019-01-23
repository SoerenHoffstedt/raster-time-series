
#ifndef RASTER_TIME_SERIES_GDAL_SOURCE_H
#define RASTER_TIME_SERIES_GDAL_SOURCE_H

#include "operators/generic_operator.h"
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
    class GDALSource : public GenericOperator {
    public:
        GDALSource(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params,
                   std::vector<std::unique_ptr<GenericOperator>> &&in);
        OptionalDescriptor nextDescriptor() override;
        void initialize() override;
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
        Resolution tileCount;

        boost::posix_time::ptime curr_time;
        boost::posix_time::ptime dataset_time_start;
        boost::posix_time::ptime dataset_time_end;

        TimeInterval time_interval;
        std::string time_format;
        std::string base_file_name;
        std::string path;
        int channel;

        /**
         * The size of the raster on disc. Is read from dataset.
         */
        Resolution fileRasterSize;
        /**
         * The extent (x1,x2,y1,y2) of the raster on disc. Is read from dataset.
         */
        SpatialReference fileRasterExtent;

        std::map<std::string, std::shared_ptr<GDALDataset>> openDatasets;

        Json::Value loadDatasetJson(const std::string &name);
        double parseIsoTime(const std::string &str) const;
        bool increaseSpatial();
        bool increaseTemporal();
        void loadCurrentGdalDataset();
        void increaseCurrentTime();
        void setCurrTimeToFirstRaster();
        double getCurrentTimeEnd() const;
    };

}

#endif //RASTER_TIME_SERIES_GDAL_SOURCE_H
