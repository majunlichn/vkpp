cmake_minimum_required(VERSION 3.10)

if ((NOT DEFINED CMAKE_TOOLCHAIN_FILE) AND (DEFINED ENV{VCPKG_ROOT}))
    list(APPEND CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()
message("CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")

project(vkpp)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(VKPP_SUBPROJECT OFF)
else()
    set(VKPP_SUBPROJECT ON)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(VKPP_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
    ${VKPP_ROOT}
)

if (NOT TARGET rad)
    add_subdirectory(imported/radcpp)
endif()

add_library(shaderc SHARED IMPORTED GLOBAL)
if (WIN32)
    set_target_properties(shaderc PROPERTIES
        IMPORTED_LOCATION $ENV{VULKAN_SDK}/Bin/shaderc_shared.dll
        IMPORTED_IMPLIB $ENV{VULKAN_SDK}/Lib/shaderc_shared.lib
    )
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
    set(VULKAN_INCLUDE_DIR ${VKPP_ROOT}/imported/Vulkan-Headers/include)
    add_subdirectory(imported/sdlpp)
endif()
add_subdirectory(vkpp)

if (NOT ${VKPP_SUBPROJECT})
    add_subdirectory(samples/VulkanViewer)
endif()
