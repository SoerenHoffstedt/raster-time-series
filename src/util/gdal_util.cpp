
#include <gdal.h>
#include "util/gdal_util.h"

using namespace rts;

std::once_flag GDALUtil::gdal_init_once;

void GDALUtil::initGdal() {
    std::call_once(gdal_init_once, [] {
        GDALAllRegister();
    });
}
