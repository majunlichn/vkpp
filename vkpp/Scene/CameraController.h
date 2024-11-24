#pragma once

#include <vkpp/Scene/Camera.h>

namespace vkpp
{

class CameraController : public rad::RefCounted<CameraController>
{
public:
    CameraController(Camera* camera);
    ~CameraController();

    struct Input
    {
        float yawRel;
        float pitchRel;
        float rollRel;
        bool moveForward;
        bool moveBack;
        bool moveLeft;
        bool moveRight;
        bool moveUp;
        bool moveDown;
    };

    void SetMoveAroundSpeed(float moveSpeed);
    void SetMoveVerticalSpeed(float moveSpeed);
    void Update(const Input& input, float deltaTime);

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

    bool IsMoving(const Input& input) const
    {
        return input.moveForward ||
            input.moveBack ||
            input.moveLeft ||
            input.moveRight ||
            input.moveUp ||
            input.moveDown;
    }

}; // class CameraController

} // namespace vkpp
