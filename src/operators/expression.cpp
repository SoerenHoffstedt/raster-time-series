
#include <iostream>
#include "expression.h"

using namespace rts;

ExpressionOperator::ExpressionOperator(Json::Value &params) : GenericOperator(params) {

}

std::unique_ptr<TimeSeries>
ExpressionOperator::createTimeSeries(std::vector<TimeSeries *> &inputs, std::shared_ptr<GenericOperator> op_ptr) {
    UniquePtrTimeSeries ts = make_unique<TimeSeries>();

    setOperands(inputs);

    if(isATimeSeries && isBTimeSeries)
        checkInputCount(inputs, 2);
    else if(isATimeSeries || isBTimeSeries)
        checkInputCount(inputs, 1);
    else
        throw std::runtime_error("Expression operator needs at least one time series as input");



    //TODO: this ignores difference in time validity of the rasters. they have to be matched.
    if(isATimeSeries){

        unsigned long min_size = operandATs->size();
        if(isBTimeSeries && operandBTs->size() < min_size)
            min_size = operandBTs->size();

        for(unsigned int i = 0; i < min_size; ++i){
            Descriptor &desc_a = operandATs->get(i);
            std::vector<Descriptor*> desc_inputs;
            desc_inputs.push_back(&desc_a);
            if(isBTimeSeries){
                Descriptor &desc_b = operandBTs->get(i);
                desc_inputs.push_back(&desc_b);
            }
            Descriptor desc(i, desc_a.res_x, desc_a.res_y, desc_a.time_start, desc_a.time_duration, desc_inputs, op_ptr);

            ts->add(desc);
        }
    }

    return ts;
}

Raster *ExpressionOperator::executeOnRaster(Descriptor *descriptor) {

    Operation operation = getOperation();

    Raster *raster_a = descriptor->getInput(0)->getRasterPtr();
    Raster *raster_b = isBTimeSeries ? descriptor->getInput(1)->getRasterPtr() : nullptr;

    Raster *raster_out = new Raster(descriptor->res_x, descriptor->res_y);

    for(int x = 0; x < descriptor->res_x; x++){
        for(int y = 0; y < descriptor->res_y; y++){
            int val = 0;

            int valA = isATimeSeries ? raster_a->getCell(x, y) : (int)operandANum;
            int valB = isBTimeSeries ? raster_b->getCell(x, y) : (int)operandBNum;

            switch(operation){
                case Operation::ADD:
                    val = valA + valB;
                    break;
                case Operation::SUB:
                    val = valA - valB;
                    break;
                case Operation::DIV:
                    val = valA / valB;
                    break;
                case Operation::MUL:
                    val = valA * valB;
                    break;
                case Operation::MOD:
                    val = valA % valB;
                    break;
            }
            raster_out->setCell(x, y, val);
        }
    }

    return raster_out;
}



void ExpressionOperator::setOperands(std::vector<TimeSeries *> &inputs) {

    Json::Value &operands = params["operands"];

    std::string operandA = operands[0]["value"].asString();
    std::string operandB = operands[1]["value"].asString();

    operandATs = nullptr;
    operandBTs = nullptr;

    if(operandA.size() == 1 && operandA[0] >= 'A' && operandA[0] <= 'Z'){
        operandATs = inputs[operandA[0] - 'A'];
    }

    if(operandB.size() == 1 && operandB[0] >= 'A' && operandB[0] <= 'Z'){
        operandBTs = inputs[operandB[0] - 'A'];
    }

    operandANum = 0.0;
    operandBNum = 0.0;

    isATimeSeries = operandATs != nullptr;
    isBTimeSeries = operandBTs != nullptr;

    if(!isATimeSeries){
        operandANum = operands[0]["value"].asDouble();
    }
    if(!isBTimeSeries){
        operandBNum = operands[1]["value"].asDouble();
    }

}

ExpressionOperator::Operation ExpressionOperator::getOperation() {
    std::string as_string = params["operation"].asString();

    if(as_string == "DIV")
        return Operation::DIV;
    else if(as_string == "MOD")
        return Operation::MOD;
    else if(as_string == "ADD")
        return Operation::ADD;
    else if(as_string == "SUB")
        return Operation::SUB;
    else if(as_string == "MUL")
        return Operation::MUL;
}
