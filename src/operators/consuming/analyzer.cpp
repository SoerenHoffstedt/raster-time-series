
#include <fstream>
#include "analyzer.h"


rts::Analyzer::Analyzer(const rts::OperatorTree *operator_tree, const rts::QueryRectangle &qrect,
                        const Json::Value &params, std::vector<std::unique_ptr<rts::GenericOperator>> &&in)
        : ConsumingOperator(operator_tree, qrect, params, std::move(in))
{
    checkInputCount(1);
}

bool rts::Analyzer::supportsOrder(rts::Order o) const {
    return o == Order::Spatial || o == Order::Temporal;
}

void rts::Analyzer::initialize() {

}

void rts::Analyzer::consume() {


    int lastTileIndex = 0;
    int rasterIndex = 0;

    std::vector<RasterStats> stats;
    stats.reserve(256);

    for(auto &input : *input_operators[0]){
        auto raster = input.getRaster();

        if(qrect.order == Order::Spatial){
            if(input.tileIndex > lastTileIndex){
                rasterIndex = 0;
            }
        }
        lastTileIndex = input.tileIndex;

        if(stats.size() < rasterIndex + 1){
            stats.emplace_back(rasterIndex);
        }

        double maximum = std::numeric_limits<double>::min();
        double minimum = std::numeric_limits<double>::max();
        double average = 0;
        auto res = raster->getResolution();

        for(int y = 0; y < res.resY; ++y){
            for(int x = 0; x < res.resX; ++x) {
                double val = raster->getCellDouble(x,y);
                average += val;
                if(maximum < val)
                    maximum = val;
                if(minimum > val)
                    minimum= val;
            }
        }

        if(stats[rasterIndex].maximum < maximum)
            stats[rasterIndex].maximum = maximum;
        if(stats[rasterIndex].minimum > minimum)
            stats[rasterIndex].minimum = minimum;

        stats[rasterIndex].cellNum += res.resX * res.resY;
        stats[rasterIndex].average += average; //((oldAvg * oldNum) + average) / stats[rasterIndex].cellNum;


        if(qrect.order == Order::Spatial){
            rasterIndex += 1;
        } else if(input.tileIndex == input.rasterTileCount - 1) {
            rasterIndex += 1;
        }
    }

    std::ofstream file(params["filename"].asString());

    for(auto &stat : stats){
        file << stat.index << ", " << (stat.average / stat.cellNum)<< ", " << stat.maximum << ", " << stat.minimum << std::endl;
    }

    file.close();
}
