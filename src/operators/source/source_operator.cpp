
#include "util/raster_calculations.h"
#include "operators/source/source_operator.h"
#include "source_operator.h"
#include "util/benchmark.h"
#include "backend/gdal_source.h"
#include "backend/fake_source.h"

using namespace rts;

SourceOperator::SourceOperator(const OperatorTree *operator_tree, const QueryRectangle &qrect, const Json::Value &params, std::vector<std::unique_ptr<GenericOperator>> &&in)
        : GenericOperator(operator_tree, qrect, params, std::move(in)), increaseDimensions(false), pixelStateX(0), pixelStateY(0), currTileIndex(0), currRasterIndex(0)
{
    const std::string &backendName = params["backend"].asString();
    if(backendName == "gdal_source"){
        backend = std::make_unique<GDALSource>(qrect, params);
    }
    else if(backendName == "fake_source"){
        backend = std::make_unique<FakeSource>(qrect, params);
    }
}


void SourceOperator::initialize(){
    backend->initialize();

    origin = backend->getOrigin();
    scale.x  = (qrect.x2 - qrect.x1) / (double)qrect.resX;
    scale.y  = (qrect.y2 - qrect.y1) / (double)qrect.resY;

    auto tileCountAndPixelStart = RasterCalculations::calculateTileCount(qrect, origin, scale);
    tileCount = tileCountAndPixelStart.first;
    rasterWorldPixelStart = tileCountAndPixelStart.second;

    setCurrTimeToFirstRaster();
}

OptionalDescriptor SourceOperator::nextDescriptor() {
    Benchmark::startSource();
    //increaseDimensions will be true normally.
    //Except at the start when first raster will be returned and variables are set correctly or
    //skipCurrentRaster/Tile is called it will also be set already.
    //else it must be increased here and for next tile increaseDimensions will be set true.
    if(increaseDimensions){
        if(qrect.order == Order::Temporal){
            if(increaseSpatially()){
                increaseTemporally();
            }
        } else if(qrect.order == Order::Spatial){
            if(increaseTemporally()){
                increaseSpatially();
            }
        }
    }
    increaseDimensions = true;

    //if currTime is bigger than query or dataset and not reset to beginning, the end of time series is reached.
    if(currTime >= qrect.t2 || currTime > backend->datasetEndTime){
        return boost::none;
    }

    //if currTileIndex is bigger than number of tiles in raster and not reset to 0, the end of time series is reached.
    if(currTileIndex >= tileCount.resX * tileCount.resY){
        return boost::none;
    }

    // starting a new raster sets pixelState variables to 0, but when raster does not start at sources origin,
    // the tile will start in negative space of the output raster. So subtract the starting pixel in world space
    // modulo the tile res to get the starting pixels in the tile that will be returned.
    if(pixelStateX == 0) {
        pixelStateX -= rasterWorldPixelStart.resX % qrect.tileRes.resX;
    }
    if(pixelStateY == 0) {
        pixelStateY -= rasterWorldPixelStart.resY % qrect.tileRes.resY;
    }

    auto ret = backend->createDescriptor(currTime, pixelStateX, pixelStateY, currTileIndex, rasterWorldPixelStart, scale, origin, tileCount);
    Benchmark::endSource();
    return ret;
}

OptionalDescriptor SourceOperator::getDescriptor(int tileIndex) {
    Benchmark::startSource();
    Resolution pixelStart = tileIndexToStartPixel(tileIndex);
    auto ret = backend->createDescriptor(currTime, pixelStart.resX, pixelStart.resY, tileIndex, rasterWorldPixelStart, scale, origin, tileCount);
    Benchmark::endSource();
    return ret;
}

Resolution SourceOperator::tileIndexToStartPixel(int tileIndex) {
    //calculate where a tile starts.
    if(tileIndex < 0 || tileIndex >= tileCount.resX * tileCount.resY)
        throw std::runtime_error("Invalid Tile index");

    //raster does not start at (0,0) if it does not align with the tiles, see explanation in nextDescriptor()
    Resolution pixelStart;
    pixelStart.resX -= rasterWorldPixelStart.resX % qrect.tileRes.resX;
    pixelStart.resY -= rasterWorldPixelStart.resY % qrect.tileRes.resY;

    for(; tileIndex > 0; --tileIndex){
        pixelStart.resX += qrect.tileRes.resX;
        if(pixelStart.resX >= qrect.resX){
            pixelStart.resX = rasterWorldPixelStart.resX % qrect.tileRes.resX;
            pixelStart.resY += qrect.tileRes.resY;
        }
    }

    return pixelStart;
}

void SourceOperator::skipCurrentRaster(const uint32_t skipCount) {
    bool spatiallyEndNotReached = true;

    for(int i = 0; i < skipCount && spatiallyEndNotReached; ++i) {


        if (qrect.order == Order::Temporal) {
            //for temporal order: reset pixel state to beginning of raster
            pixelStateX = 0;
            pixelStateY = 0;
            currTileIndex = 0;
        }
        //for both orders: advance time to next raster.
        currRasterIndex += 1;
        backend->increaseCurrentTime(currTime);
        increaseDimensions = false;

        if (qrect.order == Order::Spatial && currTime >= qrect.t2) {
            //next tile reached.
            currRasterIndex = 0;
            currTileIndex += 1;
            pixelStateX += qrect.tileRes.resX;
            if (pixelStateX >= qrect.resX) {
                pixelStateX = 0;
                pixelStateY += qrect.tileRes.resY;
            }
            setCurrTimeToFirstRaster();
            spatiallyEndNotReached = false;

        }
    }
}

void SourceOperator::skipCurrentTile(const uint32_t skipCount) {
    bool rasterEndNotReached = true;

    for(int i = 0; i < skipCount && rasterEndNotReached; ++i) {
        if (qrect.order == Order::Spatial) {
            //for spatial order: reset raster to first raster of rts
            currRasterIndex = 0;
            setCurrTimeToFirstRaster();
        }
        //for both orders: skip the tile by advancing pixel state to next tile
        pixelStateX += qrect.tileRes.resX;
        if (pixelStateX >= qrect.resX) {
            pixelStateX = 0;
            pixelStateY += qrect.tileRes.resY;
        }
        currTileIndex += 1;
        increaseDimensions = false;

        if (qrect.order == Order::Temporal && pixelStateY >= qrect.resY) {
            pixelStateX = 0;
            pixelStateY = 0;
            currTileIndex = 0;
            currRasterIndex += 1;
            backend->increaseCurrentTime(currTime);
            rasterEndNotReached = false;
        }
    }
}

bool SourceOperator::increaseTemporally() {
    backend->beforeTemporalIncrease();
    //increasing time means going to next raster.
    currRasterIndex += 1;
    backend->increaseCurrentTime(currTime);
    if(currTime >= qrect.t2 || currTime > backend->datasetEndTime){
        //end of time series or query reached.
        if(qrect.order == Order::Spatial){
            //reset to beginning of time series only if order is spatial and return true to move on to next tile
            //else keep time_curr above t2/dataset_end_time as end condition at top of nextDescriptor() method
            currRasterIndex = 0;
            setCurrTimeToFirstRaster();
            return true;
        }
    }
    return false;
}

bool SourceOperator::increaseSpatially() {
    backend->beforeSpatialIncrease();
    //increasing to next tile
    currTileIndex += 1;
    pixelStateX += qrect.tileRes.resX;
    if(pixelStateX >= static_cast<int>(qrect.resX)){
        //next tile is in next line
        pixelStateX = 0;
        pixelStateY += qrect.tileRes.resY;
        if(pixelStateY >= static_cast<int>(qrect.resY)){
            //end of raster reached.
            if(qrect.order == Order::Temporal){
                //reset only if order is temporal to get to the next raster.
                //when spatial order the end of the time series is reached, dont reset variables as end condition.
                currTileIndex = 0;
                pixelStateY = 0;
                return true;
            }
        }
    }
    return false;
}

void SourceOperator::setCurrTimeToFirstRaster() {
    //advance start point for raster, until it is not smaller than t1.
    //if currTime is already bigger than t2, the query will not return any rasters in nextDescriptor() method.
    currTime = backend->datasetStartTime;
    while(currTime < qrect.t1 && backend->getCurrentTimeEnd(currTime) <= qrect.t1){
        backend->increaseCurrentTime(currTime);
    }
}

bool SourceOperator::supportsOrder(Order order) const {
    return backend->supportsOrder(order);
}

