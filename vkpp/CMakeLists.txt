set(VKPP_SOURCE_FILES
    Core/Common.h
    Core/Common.cpp
    Core/Instance.h
    Core/Instance.cpp
    Core/PhysicalDevice.h
    Core/PhysicalDevice.cpp
    Core/Device.h
    Core/Device.cpp
    Core/Queue.h
    Core/Queue.cpp
    Core/Command.h
    Core/Command.cpp
    Core/Fence.h
    Core/Fence.cpp
    Core/Semaphore.h
    Core/Semaphore.cpp
    Core/Event.h
    Core/Event.cpp
    Core/RenderPass.h
    Core/RenderPass.cpp
    Core/Framebuffer.h
    Core/Framebuffer.cpp
    Core/Pipeline.h
    Core/Pipeline.cpp
    Core/Buffer.h
    Core/Buffer.cpp
    Core/Image.h
    Core/Image.cpp
    Core/Sampler.h
    Core/Sampler.cpp
    Core/Descriptor.h
    Core/Descriptor.cpp
    Core/Surface.h
    Core/Surface.cpp
    Core/Swapchain.h
    Core/Swapchain.cpp
    Core/ShaderCompiler.h
    Core/ShaderCompiler.cpp
    Core/Context.h
    Core/Context.cpp
    Gui/Window.h
    Gui/Window.cpp
    Gui/GuiContext.h
    Gui/GuiContext.cpp
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
    PUBLIC sdlpp
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
