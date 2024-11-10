#include <vkpp/Rendering/CameraController.h>

namespace vkpp
{

CameraController::CameraController(Camera* camera) :
    m_camera(camera)
{
    glm::vec3 direction = glm::normalize(m_camera->m_lookAt - m_camera->m_position);
    float x = direction.x;
    float y = direction.y;
    float z = direction.z;
    float r = std::sqrt(x * x + y * y + z * z);
    m_yaw = std::atan2(x, z);
    m_pitch = std::asin(y / r);
}

CameraController::~CameraController()
{
}

void CameraController::SetMoveAroundSpeed(float moveSpeed)
{
    m_moveForwardSpeed = moveSpeed;
    m_moveBackSpeed = moveSpeed;
    m_moveSideSpeed = moveSpeed;
}

void CameraController::SetMoveVerticalSpeed(float moveSpeed)
{
    m_moveUpSpeed = moveSpeed;
    m_moveDownSpeed = moveSpeed;
}

void CameraController::Update(float deltaTime)
{
    constexpr float pi = glm::pi<float>();
    if (m_input.yawRel != 0.0f)
    {
        m_yaw += m_input.yawRel * m_input.yawRelToRadians * m_yawSpeed * deltaTime;
        if (m_yaw > 2 * pi)
        {
            m_yaw -= 2.0f * pi;
        }
        else if (m_yaw < -2.0f * pi)
        {
            m_yaw += 2.0f * pi;
        }
    }
    if (m_input.pitchRel != 0.0f)
    {
        m_pitch += m_input.pitchRel * m_input.pitchRelToRadians * m_pitchSpeed * deltaTime;
        if (m_pitch > glm::radians(89.0f))
        {
            m_pitch = glm::radians(89.0f);
        }
        else if (m_pitch < glm::radians(-89.0f))
        {
            m_pitch = glm::radians(-89.0f);
        }
    }

    glm::vec3 direction;
    direction.x = std::cos(m_pitch) * std::sin(m_yaw);
    direction.y = std::sin(m_pitch);
    direction.z = std::cos(m_pitch) * std::cos(m_yaw);
    direction = glm::normalize(direction);

    glm::vec3 up = m_camera->m_up;
    glm::vec3 right = glm::cross(direction, up);
    right = glm::normalize(right);

    if (m_input.moveForward)
    {
        m_camera->m_position += direction * m_moveForwardSpeed * deltaTime;
    }
    if (m_input.moveBack)
    {
        m_camera->m_position -= direction * m_moveBackSpeed * deltaTime;
    }
    if (m_input.moveLeft)
    {
        m_camera->m_position -= right * m_moveSideSpeed * deltaTime;
    }
    if (m_input.moveRight)
    {
        m_camera->m_position += right * m_moveSideSpeed * deltaTime;
    }
    if (m_input.moveUp)
    {
        m_camera->m_position += up * m_moveUpSpeed * deltaTime;
    }
    if (m_input.moveDown)
    {
        m_camera->m_position -= up * m_moveDownSpeed * deltaTime;
    }
    m_camera->m_lookAt = m_camera->m_position + direction;

    m_input.yawRel = 0;
    m_input.pitchRel = 0;
    m_input.rollRel = 0;
}

} // namespace vkpp
