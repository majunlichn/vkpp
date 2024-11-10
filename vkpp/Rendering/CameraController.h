#pragma once

#include <vkpp/Rendering/Camera.h>

namespace vkpp
{

class CameraController : public rad::RefCounted<CameraController>
{
public:
    CameraController(Camera* camera);
    ~CameraController();

    void SetMoveAroundSpeed(float moveSpeed);
    void SetMoveVerticalSpeed(float moveSpeed);
    void Update(float deltaTime);

    Camera* m_camera;

    float m_yaw = 0;
    float m_pitch = 0;
    float m_roll = 0;

    // radians per second
    float m_yawSpeed = glm::pi<float>();
    float m_pitchSpeed = glm::pi<float>() / 2;
    float m_rollSpeed = glm::pi<float>() / 4;

    float m_moveForwardSpeed = 1.0f;
    float m_moveBackSpeed = 1.0f;
    float m_moveSideSpeed = 1.0f;
    float m_moveUpSpeed = 1.0f;
    float m_moveDownSpeed = 1.0f;

    struct Input
    {
        float yawRel;
        float yawRelToRadians;
        float pitchRel;
        float pitchRelToRadians;
        float rollRel;
        float rollRelToRadians;
        bool moveForward;
        bool moveBack;
        bool moveLeft;
        bool moveRight;
        bool moveUp;
        bool moveDown;
    } m_input = {};

    bool IsMoving() const
    {
        return m_input.moveForward ||
            m_input.moveBack ||
            m_input.moveLeft ||
            m_input.moveRight ||
            m_input.moveUp ||
            m_input.moveDown;
    }

}; // class CameraController

} // namespace vkpp
