
#include <stdexcept>
#include "util/parsing.h"

using namespace rts;

GDALDataType Parsing::parseDataType(const std::string &input) {
    if(input == "Byte")
        return GDT_Byte;
    else if(input == "UInt16")
        return GDT_UInt16;
    else if(input == "Int16")
        return GDT_Int16;
    else if(input == "UInt32")
        return GDT_UInt32;
    else if(input == "Int32")
        return GDT_Int32;
    else if(input == "Float32")
        return GDT_Float32;
    else if(input == "Float64")
        return GDT_Float64;
    else
        throw std::runtime_error("Can not parse data type from string: " + input);
}

TimeUnit Parsing::parseTimeUnit(const std::string &input) {
    if(input == "Year")
        return TimeUnit::Year;
    else if(input == "Month")
        return TimeUnit::Month;
    else if(input == "Day")
        return TimeUnit::Day;
    else if(input == "Hour")
        return TimeUnit::Hour;
    else if(input == "Minute")
        return TimeUnit::Minute;
    else if(input == "Second")
        return TimeUnit::Second;
    else
        throw std::runtime_error("Could not parse TimeUnit: " + input);
}
