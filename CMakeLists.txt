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

set(Compressonator_INSTALL_DIR "C:/Compressonator_4.5.52")

add_library(CMP_Core SHARED IMPORTED GLOBAL)
if (WIN32)
    set_target_properties(CMP_Core PROPERTIES
        IMPORTED_LOCATION ${Compressonator_INSTALL_DIR}/lib/bin/x64/CMP_Core_MT_DLL.dll
        IMPORTED_IMPLIB ${Compressonator_INSTALL_DIR}/lib/bin/x64/CMP_Core_MT_DLL.lib
    )
endif()

add_library(CMP_Framework SHARED IMPORTED GLOBAL)
if (WIN32)
    set_target_properties(CMP_Framework PROPERTIES
        IMPORTED_LOCATION ${Compressonator_INSTALL_DIR}/lib/bin/x64/CMP_Framework_MT_DLL.dll
        IMPORTED_IMPLIB ${Compressonator_INSTALL_DIR}/lib/bin/x64/CMP_Framework_MT_DLL.lib
    )
endif()

add_library(Compressonator SHARED IMPORTED GLOBAL)
if (WIN32)
    set_target_properties(Compressonator PROPERTIES
        IMPORTED_LOCATION ${Compressonator_INSTALL_DIR}/lib/bin/x64/Compressonator_MT_DLL.dll
        IMPORTED_IMPLIB ${Compressonator_INSTALL_DIR}/lib/bin/x64/Compressonator_MT_DLL.lib
    )
endif()

if (NOT TARGET sdlpp)
    set(SDLPP_BUILD_SAMPLES OFF)
    set(VULKAN_INCLUDE_DIR ${VKPP_ROOT}/imported/Vulkan-Headers/include)
    add_subdirectory(imported/sdlpp)
endif()
add_subdirectory(vkpp)

if (VKPP_BUILD_SAMPLES)
    add_subdirectory(samples/VulkanViewer)
endif()
