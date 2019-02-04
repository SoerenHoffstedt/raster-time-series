
#include <gdal_priv.h>
#include <string>
#include <filesystem>
#include "geotiff_export.h"
#include "util/gdal_util.h"
#include "util/raster_calculations.h"

using namespace std::string_literals;
using namespace rts;

using SharedGDALDataset = std::shared_ptr<GDALDataset>;

constexpr int MAX_OPEN_DATASETS = 256;

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
    SharedGDALDataset out_dataset(nullptr, GDALClose);
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

    std::map<std::string, SharedGDALDataset> sharedDatasets;
    GenericOperator *in_op = input_operators[0].get();

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

            if(qrect.order == Order::Spatial && in_desc.tileIndex > 0){
                if(sharedDatasets.find(filePath) != sharedDatasets.end()){
                    out_dataset = sharedDatasets[filePath];
                }
                else {
                    out_dataset = SharedGDALDataset((GDALDataset *) GDALOpen(filePath.c_str(), GA_Update), GDALClose);
                }
            } else {
                out_dataset = SharedGDALDataset(driver->Create(filePath.c_str(), in_desc.rasterInfo.resX, in_desc.rasterInfo.resY, 1, in_desc.dataType, papszOptions), GDALClose);
                if(qrect.order == Order::Spatial && sharedDatasets.size() < MAX_OPEN_DATASETS){
                    //temporal order only opens a dataset once and is done.
                    sharedDatasets[filePath] = out_dataset;
                }
                if(out_dataset == nullptr){
                    throw std::runtime_error("Dataset could not be opened or created.");
                }

                double scale_x = in_desc.rasterInfo.scale.x, scale_y = in_desc.rasterInfo.scale.x;
                double origin_x = in_desc.rasterInfo.x1, origin_y = in_desc.rasterInfo.y1;
                double adfGeoTransform[6]{ origin_x, scale_x, 0, origin_y, 0, scale_y };

                //set dataset parameters
                out_dataset->SetGeoTransform(adfGeoTransform);
                //TODO: set projection, this is how it's done in mapping. Projection has to be converted to WKT string.
                //std::string srs = GDAL::WKTFromCrsId(stref.crsId);
                //out_dataset->SetProjection(srs.c_str());
            }

            if(out_dataset == nullptr){
                throw std::runtime_error("Dataset could not be opened or created.");
            }
            out_rasterBand = out_dataset->GetRasterBand(1);
        }

        auto raster = in_desc.getRaster();
        void *data = raster->getVoidDataPointer();

        out_rasterBand->SetNoDataValue(in_desc.nodata);
        int x = 0, y = 0;
        int w = in_desc.tileResolution.resX, h = in_desc.tileResolution.resY;
        int offsetX = 0, offsetY = 0;

        calcTilePosAndSize(in_desc, x, y, w, h, offsetX, offsetY);

        int dataSize = raster->sizeOfDataType();

        void *data = nullptr;
        if(offsetX != 0 || offsetY != 0){
            data = raster->getVoidDataPointerOffset(offsetX, offsetY);
        } else {
            data = raster->getVoidDataPointer();
        }

        auto res = out_rasterBand->RasterIO(
                GF_Write, x, y, w, h,
                data, w, h, raster->getDataType(),
                0, dataSize * in_desc.tileResolution.resX,
                nullptr);

        if(res != CE_None){
            throw std::runtime_error("GeoTiff Export: Writing into raster failed.");
        }

        if(qrect.order == Order::Temporal && in_desc.tileIndex == in_desc.rasterTileCount - 1){
            out_dataset.reset();
            out_rasterBand = nullptr;
        }
        else if(qrect.order == Order::Spatial){
            out_dataset.reset();
            out_rasterBand = nullptr;
        }
    }
}

void GeotiffExport::calcTilePosAndSize(const Descriptor &in_desc, int &x, int &y, int &w, int &h, int &offsetX, int &offsetY) const {

    SpatialReference spatInfo = in_desc.tileSpatialInfo;

    //the origin of the single tile itself, for calculating start and
    // end pixel points of the actual data in the tile.
    Origin tileOrigin;
    tileOrigin.x = spatInfo.x1;
    tileOrigin.y = spatInfo.y1;

    //set the actual coordinates of the data in the tile, if tile is bigger than qrect.
    if(spatInfo.x1 < qrect.x1)
        spatInfo.x1 = qrect.x1;
    if(spatInfo.x2 > qrect.x2)
        spatInfo.x2 = qrect.x2;
    if(spatInfo.y1 < qrect.y1)
        spatInfo.y1 = qrect.y1;
    if(spatInfo.y2 > qrect.y2)
        spatInfo.y2 = qrect.y2;

    //start and end pixel positions of the actual data in the tile, for offset and width/height.
    auto startPixel = RasterCalculations::coordinateToPixel(in_desc.rasterInfo.scale, tileOrigin, spatInfo.x1, spatInfo.y1);
    auto endPixel   = RasterCalculations::coordinateToPixel(in_desc.rasterInfo.scale, tileOrigin, spatInfo.x2, spatInfo.y2);

    offsetX = startPixel.resX;
    offsetY = startPixel.resY;
    w = endPixel.resX - startPixel.resX;
    h = endPixel.resY - startPixel.resY;

    //x,y are the start positions of the tile data in the output raster.
    Origin rasterOrigin;
    rasterOrigin.x = in_desc.rasterInfo.x1;
    rasterOrigin.y = in_desc.rasterInfo.y1;
    auto startPixelInRaster = RasterCalculations::coordinateToPixel(in_desc.rasterInfo.scale, rasterOrigin, spatInfo.x1, spatInfo.y1);
    x = startPixelInRaster.resX;
    y = startPixelInRaster.resY;
}
