#pragma once

#include <vkpp/Core/Context.h>
#include <vkpp/Core/Math.h>

namespace vkpp
{

class Scene;

class Animation : public rad::RefCounted<Animation>
{
public:
    Animation(Scene* scene);
    ~Animation();

    Scene* m_scene;
    std::string m_name;
    double m_duration = 0.0;
    double m_ticksPerSecond = 0.0;

    struct Vec3Key
    {
        double time;
        glm::vec3 value;
    };

    struct QuatKey
    {
        double time;
        glm::fquat value;
    };

    struct Channel
    {
        std::string nodeName;
        std::vector<Vec3Key> positions;
        std::vector<QuatKey> rotations;
        std::vector<Vec3Key> scalings;
    };

}; // class Animation

} // namespace vkpp
