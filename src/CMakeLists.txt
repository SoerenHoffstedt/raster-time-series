add_executable(rts_run_query rts_run_query.cpp)
add_executable(rts_run_all_queries rts_run_all_queries.cpp)
add_executable(rts_benchmark_query benchmark_query.cpp)

include(LinkLibrariesInternal)
add_library(rts_base_lib
        datatypes/descriptor.cpp
        datatypes/raster.cpp
        datatypes/timeseries_iterator.cpp
        datatypes/spatial_temporal_reference.cpp
        util/raster_calculations.cpp
        util/gdal_util.cpp
        util/parsing.cpp
        util/expression.cpp
        util/benchmark.cpp
        util/time_interval.cpp)
target_include_directories(rts_base_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries_internal(rts_run_query rts_base_lib)
target_link_libraries_internal(rts_run_all_queries rts_base_lib)
target_link_libraries_internal(rts_benchmark_query rts_base_lib)

add_library(rts_query_lib
        queries/operator_tree.cpp)
target_link_libraries_internal(rts_query_lib rts_base_lib)
target_link_libraries_internal(rts_base_lib rts_query_lib)


add_library(rts_operators_lib
        operators/generic_operator.cpp
        operators/consuming/consuming_operator.cpp
        operators/consuming/geotiff_export.cpp
        operators/source/source_operator.cpp
        operators/source/fake_source.cpp
        operators/source/gdal_source.cpp
        operators/consuming/print.cpp
        operators/expression_operator.cpp
        operators/sampler.cpp
        operators/aggregator.cpp
        operators/cumulative_sum.cpp
        operators/convolution.cpp
        operators/order_changer.cpp
        operators/consuming/raster_value_extraction.cpp
        operators/temporal_overlap.cpp
        operators/consuming/analyzer.cpp
        operators/raster_cache.cpp)
target_link_libraries_internal(rts_operators_lib rts_base_lib)
target_link_libraries_internal(rts_base_lib rts_operators_lib)

target_link_libraries(rts_base_lib jsoncpp_lib_static)
target_include_directories(rts_base_lib PRIVATE ${JSONCPP_INCLUDE_DIRS})

find_package(GDAL REQUIRED)
target_link_libraries(rts_base_lib ${GDAL_LIBRARY})
target_include_directories(rts_base_lib PUBLIC ${GDAL_INCLUDE_DIR})

find_package(Boost COMPONENTS date_time filesystem REQUIRED)
target_link_libraries(rts_base_lib Boost::date_time Boost::filesystem)
target_include_directories(rts_base_lib PRIVATE ${Boost_INCLUDE_DIRS})

