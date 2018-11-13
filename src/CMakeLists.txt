add_executable(rts_util rts_util.cpp)

include(LinkLibrariesInternal)
add_library(rts_base_lib
        datatypes/descriptor.cpp
        datatypes/raster.cpp
        util/parameters.cpp
        datatypes/timeseries_iterator.cpp
        datatypes/spatial_temporal_reference.cpp)
target_include_directories(rts_base_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries_internal(rts_util rts_base_lib)

add_library(rts_operators_lib
        operators/generic_operator.cpp
        operators/source/fake_source.cpp
        operators/print.cpp
        operators/expression.cpp
        operators/sampler.cpp
        operators/aggregator.cpp
        operators/cumulative_sum.cpp
        operators/spatial_overlap.cpp
        operators/temporal_overlap.cpp)
target_link_libraries_internal(rts_operators_lib rts_base_lib)
target_link_libraries_internal(rts_base_lib rts_operators_lib)

add_library(rts_query_lib
        queries/query_creator.cpp)
target_link_libraries_internal(rts_query_lib rts_base_lib)
target_link_libraries_internal(rts_base_lib rts_query_lib)

target_link_libraries(rts_base_lib jsoncpp_lib_static)
target_include_directories(rts_base_lib PRIVATE ${JSONCPP_INCLUDE_DIRS})

target_link_libraries(rts_base_lib stdc++fs) #compiler option needed for using std::filesystem from C++17 with g++ 8
