
#include <gdal_priv.h>
#include <string>
#include <filesystem>
#include "geotiff_export.h"
#include "util/gdal_util.h"

using namespace std::string_literals;
using namespace rts;

GeotiffExport::GeotiffExport(QueryRectangle qrect, Json::Value &params, UniqueOperatorVector &&in)
        : ConsumingOperator(qrect, params, std::move(in))
{
    driverName = "GTiff"s;
    path = "results/"s;
}

bool GeotiffExport::supportsOrder(Order o) {
    return o == Order::Temporal;
}

void GeotiffExport::consume() {

    GDALUtil::initGdal();

    //todo: create directory.

    GDALDriver *driver = nullptr;
    GDALDataset *out_dataset = nullptr;
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

    //create file with GTiff driver of gdal if order is Temporal.
    //For spatial ordering could check if file already exists. If so, open that one with GA_Update and write into it?
    //For now only implement for Spatial
    int count = 0;
    int x = 0, y = 0;
    for(auto &in_desc : *in_op){

        if(out_dataset == nullptr){
            //new raster.
            std::string filename = path + "Raster_"s + std::to_string(count) + ".TIFF"s;
            out_dataset = driver->Create(filename.c_str(), in_desc.rasterInfo.res_x, in_desc.rasterInfo.res_y, 1, in_desc.dataType, papszOptions);

            double scale_x = 1, scale_y = -1;
            double origin_x = in_desc.rasterInfo.x1, origin_y = in_desc.rasterInfo.y1;
            double adfGeoTransform[6]{ origin_x, scale_x, 0, origin_y, 0, scale_y };

            //set dataset parameters
            out_dataset->SetGeoTransform(adfGeoTransform);
            //std::string srs = GDAL::WKTFromCrsId(stref.crsId);
            //out_dataset->SetProjection(srs.c_str());

            out_rasterBand = out_dataset->GetRasterBand(1);
            x = 0;
            y = in_desc.rasterInfo.res_y - in_desc.tileResolution.res_y; //y pixel are flipped between rts and gdal, so don't start with y = 0;
            count++;
        }

        auto raster = in_desc.getRaster();
        void *data = raster->getVoidDataPointer();

        out_rasterBand->SetNoDataValue(in_desc.nodata);
        int w = in_desc.tileResolution.res_x, h = in_desc.tileResolution.res_y;
        //TODO: what happens, when the tile has more data than to write? Its not easily handleable, i think...

        auto res = out_rasterBand->RasterIO(GF_Write, x, y, w, h, data, w, h, in_desc.dataType, 0, 0, nullptr);

        if(res != CE_None){
            throw std::runtime_error("GeoTiff Export: Writing into raster failed.");
        }

        x += w;
        if(x >= in_desc.rasterInfo.res_x){
            x = 0;
            y -= h;
        }

        if(in_desc.tileIndex == in_desc.rasterTileCount - 1){
            GDALClose(out_dataset);
            out_dataset = nullptr;
            out_rasterBand = nullptr;
        }
    }

}
