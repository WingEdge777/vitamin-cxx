cmake_minimum_required(VERSION 3.14)
project(vitamin-cxx VERSION 0.1.0 LANGUAGES CXX)

add_library(vitamin_cxx_headers INTERFACE)
target_include_directories(vitamin_cxx_headers
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_compile_features(vitamin_cxx_headers INTERFACE cxx_std_17)

# Public usage target: always link vitamin_cxx.
# When non-header sources exist, build them as STATIC and attach via INTERFACE.
add_library(vitamin_cxx INTERFACE)
target_link_libraries(vitamin_cxx INTERFACE vitamin_cxx_headers)

set(VITAMIN_CXX_SOURCES)
if(VITAMIN_CXX_SOURCES)
    add_library(vitamin_cxx_impl STATIC ${VITAMIN_CXX_SOURCES})
    target_link_libraries(vitamin_cxx_impl PUBLIC vitamin_cxx_headers)
    target_compile_options(vitamin_cxx_impl PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -Werror>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Werror>
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    )
    target_link_libraries(vitamin_cxx INTERFACE vitamin_cxx_impl)
endif()

option(VITAMIN_CXX_BUILD_EXAMPLES "Build examples" OFF)
if(VITAMIN_CXX_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
