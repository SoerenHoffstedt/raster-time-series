
#ifndef RASTER_TIME_SERIES_GEOTIFF_EXPORT_H
#define RASTER_TIME_SERIES_GEOTIFF_EXPORT_H

#include "operators/consuming/consuming_operator.h"

namespace rts {

    /**
     * Operator for exporting a raster time series as GeoTiff files to disc.
     * Params:
     *  - filename: base name for the files. They should include a %%%TIME_STRING%%% placeholder.
     *  - time_format: the format the %%%TIME_STRING%%% is replaced with. e.g. "%Y-%m-%dT%H:%M:%S" for ISO format.
     *
     */
    class GeotiffExport : public ConsumingOperator {
    public:
        explicit GeotiffExport(QueryRectangle qrect, Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in);
        void consume() override;
        void initialize() override;
        bool supportsOrder(Order o) const override;
    private:
        std::string timeFormat;
        std::string baseFilename;
        std::string driverName;
        std::string path;
        SpatialReference extent;
        /**
         * Calculates the pixel position and size of the tile. The real data might be less than the spatial information
         * of the tile, this method takes that into account by checking it against the raster spatial information.
         * The parameters are all references and will potentially be changed by calling this.
         */
        void calcTilePosAndSize(const Descriptor &desc, int &x, int &y, int &w, int &h, int &offsetX, int &offsetY) const;

    };

}




#endif //RASTER_TIME_SERIES_GEOTIFF_EXPORT_H
