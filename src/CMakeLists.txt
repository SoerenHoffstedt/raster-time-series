add_executable(rts_util rts_util.cpp)

include(LinkLibrariesInternal)
add_library(rts_base_lib
        datatypes/descriptor.cpp
        datatypes/raster.cpp
        util/parameters.cpp
        datatypes/timeseries_iterator.cpp)
target_include_directories(rts_base_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries_internal(rts_util rts_base_lib)

add_library(rts_operators_lib
        operators/generic_operator.cpp
        operators/source/fake_source.cpp
        operators/print.cpp
        operators/expression.cpp
        operators/sampler.cpp
        operators/aggregator.cpp
        operators/cumulative_sum.cpp)
target_link_libraries_internal(rts_operators_lib rts_base_lib)
target_link_libraries_internal(rts_base_lib rts_operators_lib)

add_library(rts_query_lib
        queries/query_creator.cpp)
target_link_libraries_internal(rts_query_lib rts_base_lib)
target_link_libraries_internal(rts_base_lib rts_query_lib)

target_link_libraries(rts_base_lib jsoncpp_lib_static)
target_include_directories(rts_base_lib PRIVATE ${JSONCPP_INCLUDE_DIRS})

find_package(Boost COMPONENTS date_time filesystem REQUIRED)
target_link_libraries(rts_base_lib Boost::date_time Boost::filesystem)
target_include_directories(rts_base_lib PRIVATE ${Boost_INCLUDE_DIRS})
