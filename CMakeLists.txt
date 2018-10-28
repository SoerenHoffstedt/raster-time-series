project(raster-time-series LANGUAGES CXX)

cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/target/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/target/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/target/lib)

include(DownloadProject)

download_project(PROJ       jsoncpp
        GIT_REPOSITORY      https://github.com/open-source-parsers/jsoncpp.git
        GIT_TAG             1.7.7
        UPDATE_DISCONNECTED 1
        PREFIX              CMakeFiles/Download
        )
add_subdirectory(${jsoncpp_SOURCE_DIR} ${jsoncpp_BINARY_DIR} EXCLUDE_FROM_ALL)

add_subdirectory(src)