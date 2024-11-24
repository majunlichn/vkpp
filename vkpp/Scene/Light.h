#pragma once

#include <vkpp/Core/Context.h>
#include <vkpp/Core/Math.h>

namespace vkpp
{

class Scene;

// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_lights_punctual/README.md
class Light : public rad::RefCounted<Light>
{
public:
    Light(Scene* scene);
    ~Light();

    Scene* m_scene;
    std::string m_name;
    enum class Type
    {
        Directional,
        Point,
        Spot,
    };
    Type m_type = Type::Point;

    glm::vec3 m_position = { 0, 0, 0 };
    glm::vec3 m_direction = { 0, 0, 0 };

    glm::vec3 m_color = glm::vec3(1.0f, 1.0f, 1.0f);
    float m_intensity = 1.0f;
    // These two values can be calculated on the CPU and passed into the shader:
    // float lightAngleScale = 1.0f / max(0.001f, cos(innerConeAngle) - cos(outerConeAngle));
    // float lightAngleOffset = -cos(outerConeAngle) * lightAngleScale;
    // Then, in the shader:
    // float cosAngle = dot(spotlightDir, normalizedLightVector);
    // float angularAttenuation = saturate(cosAngle * lightAngleScale + lightAngleOffset);
    // angularAttenuation *= angularAttenuation;
    float m_innerConeAngle = 0.0f;
    float m_outerConeAngle = glm::pi<float>() / 4.0f;

    // Defines a distance cutoff at which the light's intensity must be considered zero.
    // attenuation = max(min(1.0 - (d / range)^4, 1), 0) / d^2;
    float m_range = FLT_MAX;

}; // class Light

} // namespace vkpp
