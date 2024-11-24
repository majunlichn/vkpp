#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <vkpp/Scene/Camera.h>

namespace vkpp
{

Camera::Camera(Scene* scene) :
    m_scene(scene)
{
}

Camera::~Camera()
{
}

float Camera::ConvertFovH2V(float xfov, float aspectRatio)
{
    return 2.0f * std::atan(std::tan(xfov / 2.0f) / aspectRatio);
}

float Camera::ConvertFovV2H(float yfov, float aspectRatio)
{
    return 2.0f * std::atan(std::tan(yfov / 2.0f) * aspectRatio);
}

float Camera::GetHorizontalFov() const
{
    return ConvertFovV2H(m_yfov, m_aspectRatio);
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(m_position, m_lookAt, m_up);
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    if (m_type == Type::Perspective)
    {
        glm::mat4 proj = glm::perspective(m_yfov, m_aspectRatio, m_clipNear, m_clipFar);
        proj[1][1] *= -1;
        return proj;
    }
    else // if (m_type == Type::Orthographic)
    {
        // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#orthographic-projection
        float r = m_xmag / 2.0f;
        float t = m_ymag / 2.0f;
        float f = m_clipFar;
        float n = m_clipNear;
        return glm::mat4(
            1.0f / r, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f / t, 0.0f, 0.0f,
            0.0f, 0.0f, 2.0f / (n - f), 0.0f,
            0.0f, 0.0f, (f + n) / (n - f), 1.0f
        );
    }
}

glm::mat4 Camera::GetViewProjectionMatrix() const
{
    return GetProjectionMatrix() * GetViewMatrix();
}

} // namespace vkpp
