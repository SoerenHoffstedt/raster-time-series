
#include <gdal.h>
#include "util/gdal_util.h"
#include "gdal_util.h"


using namespace rts;

std::once_flag GDALUtil::gdal_init_once;

void GDALUtil::initGdal() {
    std::call_once(gdal_init_once, [] {
        GDALAllRegister();
    });
}

constexpr int MAX_STRING_LENGTH = 255;

std::string GDALUtil::timeToString(time_t time, const std::string &timeFormat) {
    char date[MAX_STRING_LENGTH] = {0};
    tm curr_time_tm = *gmtime(&time);
    strftime(date, sizeof(date), timeFormat.c_str(), &curr_time_tm);
    std::string timeString(date);
    return timeString;
}
