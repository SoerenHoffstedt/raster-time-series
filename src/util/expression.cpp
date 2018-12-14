
#include "datatypes/raster_operations.h"
#include "util/expression.h"
#include "expression.h"


using namespace rts;
using namespace std::string_literals;

template<class T1, class T2, class T3>
struct BinaryExpression {
    static void rasterOperation(TypedRaster<T1> *output, TypedRaster<T2> *input1, TypedRaster<T3> *input2, Expression::Operator op){
        Resolution tileSize = output->getResolution();
        for (int x = 0; x < tileSize.res_x; ++x) {
            for (int y = 0; y < tileSize.res_y; ++y) {
                T1 val = 0;
                switch(op){
                    case Expression::Operator::ADD:
                        val = (T1)(input1->getCell(x, y) + input2->getCell(x, y));
                        break;
                    case Expression::Operator::SUB:
                        val = (T1)(input1->getCell(x, y) - input2->getCell(x, y));
                        break;
                    case Expression::Operator::DIV:
                        val = (T1)(input1->getCell(x, y) / input2->getCell(x, y));
                        break;
                    case Expression::Operator::MUL:
                        val = (T1)(input1->getCell(x, y) * input2->getCell(x, y));
                        break;
                    case Expression::Operator::MOD:
                        //TODO: ints
                        val = (T1)((int)input1->getCell(x, y) % (int)input2->getCell(x, y));
                        break;
                }
                output->setCell(x, y, val);
            }
        }

    }
};

template<class T1, class T2>
struct UnaryExpressionRasterFirst {
    static void rasterOperation(TypedRaster<T1> *output, TypedRaster<T2> *input1, double second_operand, Expression::Operator op){
        Resolution tileSize = output->getResolution();
        for (int x = 0; x < tileSize.res_x; ++x) {
            for (int y = 0; y < tileSize.res_y; ++y) {
                T1 val = 0;
                switch(op){
                    case Expression::Operator::ADD:
                        val = (T1)(input1->getCell(x, y) + second_operand);
                        break;
                    case Expression::Operator::SUB:
                        val = (T1)(input1->getCell(x, y) - second_operand);
                        break;
                    case Expression::Operator::DIV:
                        val = (T1)(input1->getCell(x, y) / second_operand);
                        break;
                    case Expression::Operator::MUL:
                        val = (T1)(input1->getCell(x, y) * second_operand);
                        break;
                    case Expression::Operator::MOD:
                        //TODO: ints
                        val = (T1)((int)input1->getCell(x, y) % (int)second_operand);
                        break;
                }
                output->setCell(x, y, val);
            }
        }

    }
};

template<class T1, class T2>
struct UnaryExpressionRasterSecond {
    static void rasterOperation(TypedRaster<T1> *output, TypedRaster<T2> *input1, double first_operand, Expression::Operator op){
        Resolution tileSize = output->getResolution();
        for (int x = 0; x < tileSize.res_x; ++x) {
            for (int y = 0; y < tileSize.res_y; ++y) {
                T1 val = 0;
                switch(op){
                    case Expression::Operator::ADD:
                        val = (T1)(first_operand + input1->getCell(x, y));
                        break;
                    case Expression::Operator::SUB:
                        val = (T1)(first_operand - input1->getCell(x, y));
                        break;
                    case Expression::Operator::DIV:
                        val = (T1)(first_operand / input1->getCell(x, y));
                        break;
                    case Expression::Operator::MUL:
                        val = (T1)(first_operand * input1->getCell(x, y));
                        break;
                    case Expression::Operator::MOD:
                        //TODO: remove int
                        val = (T1)((int)first_operand % (int)input1->getCell(x, y));
                        break;
                }
                output->setCell(x, y, val);
            }
        }

    }
};

Expression::Expression(const Json::Value &def) : Expression(def.asString()) { }

Expression::Expression(const std::string &expr) {

    size_t pos = std::string::npos;

    if((pos = expr.find('+')) != std::string::npos)
        op = Operator::ADD;
    else if((pos = expr.find('-')) != std::string::npos)
        op = Operator::SUB;
    else if((pos = expr.find('*')) != std::string::npos)
        op = Operator::MUL;
    else if((pos = expr.find('/')) != std::string::npos)
        op = Operator::DIV;
    else if((pos = expr.find('%')) != std::string::npos)
        op = Operator::MOD;

    if(pos == std::string::npos){
        throw std::runtime_error("No operator found in expression string.");
    }

    //get substring before and after operator and trim it from whitespace.
    std::string first = expr.substr(0, pos);
    first.erase(first.begin(), std::find_if(first.begin(), first.end(), [](char ch){ return !std::isspace(ch); }));
    firstOperand = Operand::createFromString(first);

    std::string second = expr.substr(pos + 1, std::string::npos);
    second.erase(second.begin(), std::find_if(second.begin(), second.end(), [](char ch){ return !std::isspace(ch); }));
    secondOperand = Operand::createFromString(second);

    expectedInputs = 0;
    if(firstOperand.type == OperandType::Raster)
        expectedInputs += 1;
    if(secondOperand.type == OperandType::Raster)
        expectedInputs += 1;
    if(expectedInputs == 0)
        throw std::runtime_error("Invalid expression (no rasters inserted): "s + expr);
}

std::function<UniqueRaster(const Descriptor&)> Expression::createGetter(std::vector<OptionalDescriptor> &&inputs) const {

    if(inputs.size() != expectedInputs){
        throw std::runtime_error("Received inputs do not match the expected inputs.");
    }

    if(expectedInputs == 2){
        std::function<UniqueRaster(const Descriptor&)> getter = [inputs = std::move(inputs),
                                                                 op = op,
                                                                 firstIndex = firstOperand.rasterIndex,
                                                                 secondIndex = secondOperand.rasterIndex](const Descriptor &self) -> UniqueRaster
        {
            auto output = Raster::createRaster(self.dataType, self.tileResolution);
            UniqueRaster rasterA = inputs[firstIndex]->getRaster();
            UniqueRaster rasterB = inputs[secondIndex]->getRaster();
            RasterOperations::callTernary<BinaryExpression>(output.get(), rasterA.get(), rasterB.get(), op);
            return output;
        };

        return getter;
    } else
    {
        std::function<UniqueRaster(const Descriptor&)> getter = [inputs = std::move(inputs),
                                                                 &firstOperand = firstOperand,
                                                                 &secondOperand = secondOperand,
                                                                 op = op] (const Descriptor &self) -> UniqueRaster
         {
            auto output = Raster::createRaster(self.dataType, self.tileResolution);
            auto input = inputs[0]->getRaster();

            if(firstOperand.type == OperandType::Number)
                RasterOperations::callBinary<UnaryExpressionRasterSecond>(output.get(), input.get(), firstOperand.numericValue, op);
            else if(secondOperand.type == OperandType::Number)
                RasterOperations::callBinary<UnaryExpressionRasterFirst>(output.get(), input.get(), secondOperand.numericValue, op);

            return output;
        };
        return getter;
    }

}

//Operand
Expression::Operand Expression::Operand::createRasterOperand(int rasterIndex) {
    Operand o{};
    o.type = OperandType::Raster;
    o.rasterIndex = rasterIndex;
    return o;
}

Expression::Operand Expression::Operand::createNumberOperand(double numericValue) {
    Operand o{};
    o.type = OperandType::Number;
    o.numericValue = numericValue;
    return o;
}

Expression::Operand Expression::Operand::createFromString(std::string &str) {

    double numeric = std::strtod(str.c_str(), nullptr);
    if(numeric != 0.0){
        return Operand::createNumberOperand(numeric);
    } else {
        char r = str[0];
        if(r != 'A' && r != 'B')
            throw std::runtime_error("Invalid letter for a raster. Or double parsing failed. Number can not be 0.");
        int index = r == 'A' ? 0 : 1;
        return Operand::createRasterOperand(index);
    }
}
