cmake_minimum_required(VERSION 3.10)

if (DEFINED ENV{VCPKG_ROOT})
    list(APPEND CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()
message("CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")

project(vkpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(VKPP_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
    ${VKPP_ROOT}
)

add_subdirectory(imported/radcpp)

add_library(spvgen SHARED IMPORTED GLOBAL)
if (WIN32)
    set_target_properties(spvgen PROPERTIES
        IMPORTED_LOCATION "${RADCPP_ROOT}/imported/spvgen/build/Release/spvgen.dll"
        IMPORTED_IMPLIB "${RADCPP_ROOT}/imported/spvgen/build/Release/spvgen.lib"
    )
endif()

add_subdirectory(vkpp)
add_subdirectory(samples/VulkanViewer)
