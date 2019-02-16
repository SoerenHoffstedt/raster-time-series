
#include "util/raster_calculations.h"
#include "util/benchmark.h"
#include "raster_value_extraction.h"
#include <fstream>

using namespace rts;

RasterValueExtraction::RasterValueExtraction(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, UniqueOperatorVector &&in)
        : ConsumingOperator(operator_tree, qrect, params, std::move(in))
{
    checkInputCount(1);
}

void RasterValueExtraction::initialize() {
    std::string outputString = params["output"].asString();
    if(outputString == "print")
        output = Output::Print;
    else if(outputString == "file")
        output = Output::File;
}

bool RasterValueExtraction::supportsOrder(Order order) const {
    return order == Order::Spatial || order == Order::Temporal;
}

void RasterValueExtraction::consume() {

    Benchmark::startConsuming();
    OptionalDescriptor desc = input_operators[0]->nextDescriptor();
    if(desc == std::nullopt)
        return;
    //initialize the stuff!!
    SpatialReference extent = desc->rasterInfo.projection.getExtent();
    Resolution tileSize = desc->tileResolution;
    SpatialReference tileSizeSpat;
    tileSizeSpat.x1 = 0;
    tileSizeSpat.x2 = desc->tileSpatialInfo.x2 - desc->tileSpatialInfo.x1;
    tileSizeSpat.y1 = 0;
    tileSizeSpat.y2 = desc->tileSpatialInfo.y2 - desc->tileSpatialInfo.y1;

    std::vector<TemporalPoint> points;
    auto pointsJson = params["points"];
    bool isArray = pointsJson.isArray();
    uint64_t size = points.size();
    points.reserve(size);
    for(auto &pJson : pointsJson){
        points.emplace_back(pJson, tileSizeSpat, extent);
    }
    //for(int i = 0; i < size; ++i){
    //    points.emplace_back(pointsJson[i], tileSizeSpat, extent);
    //}

    //sort the points
    if(qrect.order == Order::Temporal)
        std::sort(points.begin(), points.end(), TemporalPoint::comparerTemporal);
    else
        std::sort(points.begin(), points.end(), TemporalPoint::comparerSpatial);

    //TODO: check if time overlaps with raster, and if poiints are inside the raster.
    std::ofstream file_output;
    if(output == Output::File){
        //TODO: if results dir does not exist: create it.
        std::string filename = params["filename"].asString();
        file_output.open("results/ " + filename);
    }

    UniqueRaster raster = nullptr;
    int rasterLoadCount = 0;

    Benchmark::endConsuming();
    //iterate points!
    for(int i = 0; i < points.size(); ++i){
        Benchmark::startConsuming();
        auto &point = points[i];
        //TODO: solve: what happens when
        /*
         * point.t < desc.t1 & t2: keep desc, next point.
         * point.t > desc.t1 & t2: keep point, next desc.
         * desc.t1 < point.t < desc.t2: print pixel value, keep desc.
         */

        //point is valid before desc, so skip point (or print no value??)
        if(point.t < desc->rasterInfo.t1 && point.t < desc->rasterInfo.t2){
            std::cout << "(" << point.x << "," << point.y << ") at [" << point.t << "] : ";
            std::cout << "No valid raster at this point." << std::endl;
            continue;
        }

        //if descriptor is not valid at time of point or point is outside desc tile: load next descsriptor.
        while(!desc->rasterInfo.containsTemporal(point.t) || !desc->tileSpatialInfo.containsSpatial(point.x, point.y)){
            desc = input_operators[0]->nextDescriptor();
            raster = nullptr;
            if(desc == std::nullopt){
                std::cout << "(" << point.x << "," << point.y << ") at [" << point.t << "] : ";
                std::cout << "No valid raster at this point." << std::endl;
                return;
            }
        }

        Resolution pixelCoord = RasterCalculations::coordinateToPixel(desc->rasterInfo.scale, desc->rasterInfo.projection.getOrigin(), point.x, point.y);

        pixelCoord.resX = pixelCoord.resX % desc->tileResolution.resX;
        pixelCoord.resY = pixelCoord.resY % desc->tileResolution.resY;
        if(raster == nullptr){
            Benchmark::endConsuming();
            raster = desc->getRaster();
            rasterLoadCount++;
            Benchmark::startConsuming();
        }
        if(output == Output::Print){
            std::cout << "(" << point.x << "," << point.y << ") at [" << point.t << "] : ";
            printPixelAt(std::cout, raster.get(), pixelCoord.resX, pixelCoord.resY);
        } else {
            file_output << "(" << point.x << "," << point.y << ") at [" << point.t << "] : ";
            printPixelAt(file_output, raster.get(), pixelCoord.resX, pixelCoord.resY);
        }
        Benchmark::endConsuming();
    }
    std::cout << "tiles loaded: " << rasterLoadCount << std::endl;
}

void RasterValueExtraction::printPixelAt(std::ostream &output, Raster *raster, int x, int y) const {
    switch(raster->getDataType()){
        case GDT_Byte:
            output << (int)(((TypedRaster<unsigned char>*)raster)->getCell(x,y)) << std::endl;
            break;
        case GDT_UInt16:
            output << ((TypedRaster<uint16_t>*)raster)->getCell(x,y) << std::endl;
            break;
        case GDT_Int16:
            output << ((TypedRaster<int16_t>*)raster)->getCell(x,y) << std::endl;
            break;
        case GDT_UInt32:
            output << ((TypedRaster<uint32_t>*)raster)->getCell(x,y) << std::endl;
            break;
        case GDT_Int32:
            output << ((TypedRaster<int32_t>*)raster)->getCell(x,y) << std::endl;
            break;
        case GDT_Float32:
            output << ((TypedRaster<float>*)raster)->getCell(x,y) << std::endl;
            break;
        case GDT_Float64:
            output << ((TypedRaster<double>*)raster)->getCell(x,y) << std::endl;
            break;
        default:
            break;
    }
}

// TemporalPoint

TemporalPoint::TemporalPoint(const Json::Value &def, SpatialReference &tileSizeSpat, SpatialReference &extent) {
    x = def["x"].asDouble();
    y = def["y"].asDouble();
    t = def["t"].asDouble();

    calcAndSetTileIndex(tileSizeSpat, extent);
}

TemporalPoint::TemporalPoint(double x, double y, double t, SpatialReference &tileSizeSpat, SpatialReference &extent)
        : x(x), y(y), t(t)
{
    calcAndSetTileIndex(tileSizeSpat, extent);
}

//TODO: if tileIndex is the same, how to decide? Simply smaller y value, than smaller x value?
bool TemporalPoint::comparerTemporal(const TemporalPoint &p1, const TemporalPoint &p2) {
    return p1.t < p2.t || (p1.t == p2.t && p1.tileIndex < p2.tileIndex);
}

bool TemporalPoint::comparerSpatial(const TemporalPoint &p1, const TemporalPoint &p2) {
    return p1.tileIndex < p2.tileIndex || (p1.tileIndex == p2.tileIndex && p1.t < p2.t);
}

void TemporalPoint::calcAndSetTileIndex(SpatialReference &tileSizeSpat, SpatialReference &extent) {
    //TODO: tile index must be calculated from the qrect of the raster, not the whole extent!
    tileIndex = 0;
    SpatialReference cutout(extent.x1, extent.x1 + tileSizeSpat.x2, extent.y1, extent.y1 + tileSizeSpat.y2);

    while(cutout.overlapsWithSpatial(extent) && !cutout.containsSpatial(x, y)){
        tileIndex += 1;
        cutout.x1 += tileSizeSpat.x2;
        cutout.x2 += tileSizeSpat.x2;

        if(cutout.x1 > extent.x2){
            cutout.x1 = extent.x1;
            cutout.x2 = extent.x1 + tileSizeSpat.x2;
            cutout.y1 += tileSizeSpat.y2;
            cutout.y2 += tileSizeSpat.y2;
        }
    }

    if(!cutout.containsSpatial(x, y))
        throw std::runtime_error("Point is not on raster.");
}
