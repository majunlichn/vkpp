set(VKPP_SOURCE_FILES
    Core/Common.h
    Core/Common.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VKPP_SOURCE_FILES})

add_library(vkpp STATIC ${VKPP_SOURCE_FILES})

target_include_directories(vkpp
    PUBLIC ${VKPP_ROOT}/imported/radcpp
    PUBLIC ${VKPP_ROOT}/imported/Vulkan-Headers/include
    PUBLIC ${VKPP_ROOT}/imported/volk
    PUBLIC ${VKPP_ROOT}/imported/Vulkan-Utility-Libraries/include
    PUBLIC ${VKPP_ROOT}/imported/VulkanMemoryAllocator/include
    PUBLIC ${VKPP_ROOT}/imported/spvgen/include
)

target_link_libraries(vkpp
    PUBLIC rad
)

if (WIN32)
target_compile_definitions(vkpp
    PRIVATE NOMINMAX=1
    PRIVATE WIN32_LEAN_AND_MEAN=1
)
target_compile_options(vkpp
    PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)
endif()
