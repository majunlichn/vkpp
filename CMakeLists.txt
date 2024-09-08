cmake_minimum_required(VERSION 3.10)

if (DEFINED ENV{VCPKG_ROOT})
    list(APPEND CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()
message("CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")

project(vkpp)

option(VKPP_BUILD_SAMPLES "Build samples." ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(VKPP_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
    ${VKPP_ROOT}
)

if (NOT TARGET rad)
    set(RAD_BUILD_TESTS OFF)
    add_subdirectory(imported/radcpp)
endif()

add_library(spvgen SHARED IMPORTED GLOBAL)
if (WIN32)
    set_target_properties(spvgen PROPERTIES
        IMPORTED_LOCATION "${RADCPP_ROOT}/imported/spvgen/build/Release/spvgen.dll"
        IMPORTED_IMPLIB "${RADCPP_ROOT}/imported/spvgen/build/Release/spvgen.lib"
    )
endif()

if (NOT TARGET sdlpp)
    set(SDLPP_BUILD_SAMPLES OFF)
    add_subdirectory(imported/sdlpp)
endif()
add_subdirectory(vkpp)

if (VKPP_BUILD_SAMPLES)
    add_subdirectory(samples/VulkanViewer)
endif()
