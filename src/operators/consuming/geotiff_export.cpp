
#include <gdal_priv.h>
#include <string>
#include <filesystem>
#include "geotiff_export.h"
#include "util/gdal_util.h"
#include "util/raster_calculations.h"

using namespace std::string_literals;
using namespace rts;

using UniqueGDALDataset = std::unique_ptr<GDALDataset, decltype(&GDALClose)>;

GeotiffExport::GeotiffExport(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, UniqueOperatorVector &&in)
        : ConsumingOperator(operator_tree, qrect, params, std::move(in))
{
    checkInputCount(1);
}

void GeotiffExport::initialize() {
    driverName      = "GTiff"s;
    path            = "results/"s;
    extent          = qrect.projection.getExtent();
    timeFormat      = params["time_format"].asString();
    baseFilename    = params["filename"].asString();
}

bool GeotiffExport::supportsOrder(Order o) const {
    return o == Order::Temporal || o == Order::Spatial;
}

void GeotiffExport::consume() {

    GDALUtil::initGdal();

    GDALDriver *driver = nullptr;
    UniqueGDALDataset out_dataset(nullptr, GDALClose);
    GDALRasterBand *out_rasterBand = nullptr;

    driver = GetGDALDriverManager()->GetDriverByName(driverName.c_str());

    if(driver == nullptr){
        throw std::runtime_error("Driver could not be found: "s + driverName);
    }

    char **papszOptions = nullptr;

    if (strcmp(driverName.c_str(), "GTiff") == 0) {
        papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "DEFLATE");
    }

    std::filesystem::path p(path.c_str());
    if(!std::filesystem::exists(p)){
        std::filesystem::create_directory(p);
    }

    GenericOperator *in_op = input_operators[0].get();

    int x = 0, y = 0;
    for(auto &in_desc : *in_op){

        if(out_dataset == nullptr){
            //new raster.
            std::string timeString = GDALUtil::timeToString(static_cast<time_t>(in_desc.rasterInfo.t1), timeFormat);
            std::string placeholder = "%%%TIME_STRING%%%";
            size_t placeholderPos = baseFilename.find(placeholder);
            std::string filename = baseFilename;
            filename.replace(placeholderPos, placeholder.length(), timeString);
            std::filesystem::path filePath = path;
            filePath /= filename;

            //GDALDatasets are not kept open at the moment for spatial order.
            if(qrect.order == Order::Spatial && in_desc.tileIndex > 0){
                out_dataset = UniqueGDALDataset((GDALDataset *)GDALOpen(filePath.c_str(), GA_Update), GDALClose);
            } else {
                out_dataset = UniqueGDALDataset(driver->Create(filePath.c_str(), in_desc.rasterInfo.res_x, in_desc.rasterInfo.res_y, 1, in_desc.dataType, papszOptions), GDALClose);
            }

            if(out_dataset == nullptr){
                throw std::runtime_error("Dataset could not be opened or created.");
            }

            double scale_x = 1, scale_y = -1;
            double origin_x = in_desc.rasterInfo.x1, origin_y = in_desc.rasterInfo.y1;
            double adfGeoTransform[6]{ origin_x, scale_x, 0, origin_y, 0, scale_y };

            //set dataset parameters
            out_dataset->SetGeoTransform(adfGeoTransform);
            //TODO: set projection, this is how it's done in mapping. Projection has to be converted to WKT string.
            //std::string srs = GDAL::WKTFromCrsId(stref.crsId);
            //out_dataset->SetProjection(srs.c_str());

            out_rasterBand = out_dataset->GetRasterBand(1);
        }

        auto raster = in_desc.getRaster();
        void *data = raster->getVoidDataPointer();

        out_rasterBand->SetNoDataValue(in_desc.nodata);
        int w = in_desc.tileResolution.res_x, h = in_desc.tileResolution.res_y;
        int offsetX = 0, offsetY = 0;

        calcTilePosAndSize(in_desc, x, y, w, h, offsetX, offsetY);

        int dataSize = raster->sizeOfDataType();

        if(offsetX != 0 || offsetY != 0){
            data = raster->getVoidDataPointerOffset(offsetX, offsetY);
        }

        auto res = out_rasterBand->RasterIO(
                GF_Write, x, y, w, h,
                data, w, h, raster->getDataType(),
                0, dataSize * in_desc.tileResolution.res_x,
                nullptr);

        if(res != CE_None){
            throw std::runtime_error("GeoTiff Export: Writing into raster failed.");
        }

        if(qrect.order == Order::Temporal && in_desc.tileIndex == in_desc.rasterTileCount - 1){
            out_dataset.reset(nullptr);
            out_rasterBand = nullptr;
        }
        if(qrect.order == Order::Spatial){
            //TODO: dont do it here, but above. If its spatial, but only one raster (eg aggregator over complete time series) it does not have to be closed.
            out_dataset.reset(nullptr);
            out_rasterBand = nullptr;
        }
    }
}

void GeotiffExport::calcTilePosAndSize(const Descriptor &in_desc, int &x, int &y, int &w, int &h, int &offsetX, int &offsetY) const {

    //The spatial extent of a tile can be more that the query rectangles. Here only the really requested data
    //should be written, so make the tile spatial info smaller if needed.
    SpatialReference spatInfo = in_desc.tileSpatialInfo;
    if(spatInfo.x1 < qrect.x1)
        spatInfo.x1 = qrect.x1;
    if(spatInfo.x2 > qrect.x2)
        spatInfo.x2 = qrect.x2;
    if(spatInfo.y1 < qrect.y1)
        spatInfo.y1 = qrect.y1;
    if(spatInfo.y2 > qrect.y2)
        spatInfo.y2 = qrect.y2;

    Resolution start = RasterCalculations::coordinateToPixel(in_desc.rasterInfo, spatInfo.x1, spatInfo.y1);
    Resolution end   = RasterCalculations::coordinateToPixel(in_desc.rasterInfo, spatInfo.x2, spatInfo.y2);

    x = start.res_x;
    y = start.res_y;

    int modXStart   = start.res_x % in_desc.tileResolution.res_x;
    int modYStart   = start.res_y % in_desc.tileResolution.res_y;
    int modXEnd     = end.res_x % in_desc.tileResolution.res_x;
    int modYEnd     = end.res_y % in_desc.tileResolution.res_y;

    //if start pixel are not aligned to the tile resolution, an offset for the data pointer is needed:
    offsetX = modXStart;
    offsetY = modYStart;

    if(modXStart != 0)
        w -= modXStart;
    if(modYStart != 0)
        h -= modYStart;

    if(modXEnd  != 0)
        w -= in_desc.tileResolution.res_x - modXEnd;
    if(modYEnd != 0)
        h -= in_desc.tileResolution.res_y - modYEnd;

}
