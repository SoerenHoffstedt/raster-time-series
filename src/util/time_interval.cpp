
#include "util/parsing.h"
#include "time_interval.h"

using namespace rts;
using namespace boost::posix_time;

TimeInterval::TimeInterval() : unit(TimeUnit::Year), length(0) { }

TimeInterval::TimeInterval(TimeUnit unit, uint32_t length) : unit(unit), length(length) { }

TimeInterval::TimeInterval(const Json::Value &json) {
    unit   = Parsing::parseTimeUnit(json["unit"].asString());
    length = json["length"].asUInt();
}

void TimeInterval::increase(boost::posix_time::ptime &time, uint32_t times) const {
    switch(unit){
        case TimeUnit::Year:
            time += boost::gregorian::years(length * times);
            break;
        case TimeUnit::Month:
            time += boost::gregorian::months(length * times);
            break;
        case TimeUnit::Day:
            time += boost::gregorian::days(length * times);
            break;
        case TimeUnit::Hour:
            time += boost::posix_time::hours(length * times);
            break;
        case TimeUnit::Minute:
            time += boost::posix_time::minutes(length * times);
            break;
        case TimeUnit::Second:
            time += boost::posix_time::seconds(length * times);
            break;
    }
}

void TimeInterval::decrease(boost::posix_time::ptime &time, uint32_t times) const {
    switch(unit){
        case TimeUnit::Year:
            time -= boost::gregorian::years(length * times);
            break;
        case TimeUnit::Month:
            time -= boost::gregorian::months(length * times);
            break;
        case TimeUnit::Day:
            time -= boost::gregorian::days(length * times);
            break;
        case TimeUnit::Hour:
            time -= boost::posix_time::hours(length * times);
            break;
        case TimeUnit::Minute:
            time -= boost::posix_time::minutes(length * times);
            break;
        case TimeUnit::Second:
            time -= boost::posix_time::seconds(length * times);
            break;
    }
}
