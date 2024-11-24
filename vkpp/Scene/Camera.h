#pragma once

#include <vkpp/Core/Context.h>
#include <vkpp/Core/Math.h>

namespace vkpp
{

class Scene;

class Camera : public rad::RefCounted<Camera>
{
public:
    Camera(Scene* scene);
    ~Camera();

    Scene* m_scene;
    std::string m_name;
    enum class Type
    {
        Perspective,
        Orthographic,
    };
    Type m_type = Type::Perspective;

    glm::vec3 m_position = { 0, 0, 0 };
    glm::vec3 m_lookAt = { 0, 0, 1 };
    glm::vec3 m_up = { 0, 1, 0 };

    // Aspect ratio of the field of view.
    float m_aspectRatio = 0.0f;
    // Vertical field of view in radians.
    float m_yfov = glm::radians(45.0f);
    // Distance to the near clipping plane.
    float m_clipNear = 0.1f;
    // Distance to the far clipping plane.
    float m_clipFar = 1000.0f;

    // Horizontal magnification of the view, half the orthographic width.
    float m_xmag = 0.0f;
    // Vertical magnification of the view, half the orthographic height.
    float m_ymag = 0.0f;

    // When xmag/ymag does not match the aspect ratio of the viewport,
    // client implementations SHOULD NOT crop or perform non-uniform scaling ("stretching")
    // to fill the viewport.

    static float ConvertFovH2V(float xfov, float aspectRatio);
    static float ConvertFovV2H(float yfov, float aspectRatio);
    float GetHorizontalFov() const;

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
    glm::mat4 GetViewProjectionMatrix() const;

}; // class Camera

} // namespace vkpp
