set(VKPP_SOURCE_FILES
    Core/Common.h
    Core/Common.cpp
    Core/Math.h
    Core/Math.cpp
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
    Core/ShaderIncluder.h
    Core/Context.h
    Core/Context.cpp
    Gui/Window.h
    Gui/Window.cpp
    Gui/GuiContext.h
    Gui/GuiContext.cpp
    Rendering/Mesh.h
    Rendering/Mesh.cpp
    Rendering/Material.h
    Rendering/Material.cpp
    Rendering/Camera.h
    Rendering/Camera.cpp
    Rendering/Light.h
    Rendering/Light.cpp
    Rendering/Scene.h
    Rendering/Scene.cpp
    Rendering/SceneNode.h
    Rendering/SceneNode.cpp
    Rendering/SceneImporter.h
    Rendering/SceneImporter.cpp
    Rendering/SolidRenderer.h
    Rendering/SolidRenderer.cpp
    Shaders/Common/Hash.glsl
    Shaders/Gui/BlitToSwapchain.vert
    Shaders/Gui/BlitToSwapchain.frag
    Shaders/Rendering/Solid.glsl
    Shaders/Rendering/Solid.vert
    Shaders/Rendering/Solid.frag
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VKPP_SOURCE_FILES})

add_library(vkpp STATIC ${VKPP_SOURCE_FILES})

target_include_directories(vkpp
    PUBLIC ${VKPP_ROOT}/imported/radcpp
    PUBLIC ${VKPP_ROOT}/imported/Vulkan-Headers/include
    PUBLIC ${VKPP_ROOT}/imported/volk
    PUBLIC ${VKPP_ROOT}/imported/Vulkan-Utility-Libraries/include
    PUBLIC ${VKPP_ROOT}/imported/VulkanMemoryAllocator/include
    PUBLIC $ENV{VULKAN_SDK}/Include
)

target_link_libraries(vkpp
    PUBLIC rad
    PUBLIC sdlpp
)

find_package(glm CONFIG REQUIRED)
target_link_libraries(vkpp PUBLIC glm::glm)

target_include_directories(vkpp
    PUBLIC ${Compressonator_INSTALL_DIR}/include
)

target_link_libraries(vkpp
    PUBLIC shaderc
)

target_link_libraries(vkpp
    PUBLIC CMP_Core CMP_Framework Compressonator
)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(vkpp PRIVATE assimp::assimp)

if (WIN32)
target_compile_definitions(vkpp
    PRIVATE NOMINMAX=1
    PRIVATE WIN32_LEAN_AND_MEAN=1
)
target_compile_options(vkpp
    PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)
endif()
