#pragma once

#include <vkpp/Core/Instance.h>
#include <vkpp/Core/PhysicalDevice.h>
#include <vkpp/Core/Device.h>
#include <vkpp/Core/Queue.h>
#include <vkpp/Core/Command.h>
#include <vkpp/Core/Fence.h>
#include <vkpp/Core/Semaphore.h>
#include <vkpp/Core/Event.h>
#include <vkpp/Core/RenderPass.h>
#include <vkpp/Core/Framebuffer.h>
#include <vkpp/Core/Pipeline.h>
#include <vkpp/Core/Buffer.h>
#include <vkpp/Core/Image.h>
#include <vkpp/Core/Sampler.h>
#include <vkpp/Core/Descriptor.h>
#include <vkpp/Core/Surface.h>
#include <vkpp/Core/Swapchain.h>

namespace vkpp
{

class Context : public rad::RefCounted<Context>
{
public:
    Context();
    virtual ~Context();
    VKPP_DISABLE_COPY_AND_MOVE(Context);

    bool Init(rad::Ref<Instance> instance, rad::Ref<PhysicalDevice> gpu);
    Instance* GetInstance() { return m_instance.get(); }
    Device* GetDevice() { return m_device.get(); }
    Queue* GetQueue(QueueFamily family = QueueFamilyUniversal)
    {
        return m_queues[family].get();
    }

    rad::Ref<Instance> m_instance;
    rad::Ref<Device> m_device;
    rad::Ref<Queue> m_queues[QueueFamilyCount];

}; // class Context

} // namespace vkpp
