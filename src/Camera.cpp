#include "Camera.hpp"

#include <cstdio>
#include <glm/glm.hpp>

namespace gps {

    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        position       = cameraPosition;
        frontDirection = glm::normalize(cameraTarget - cameraPosition);
        upDirection    = glm::normalize(cameraUp);
        rightDirection = glm::normalize(glm::cross(frontDirection, upDirection));
    }

    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(position, position + frontDirection, upDirection);
    }

    void Camera::move(MOVE_DIRECTION direction, float speed) {
        switch (direction) {
            case MOVE_FORWARD:
                position += frontDirection * speed;
                break;

            case MOVE_BACKWARD:
                position -= frontDirection * speed;
                break;

            case MOVE_RIGHT:
                position += rightDirection * speed;
                break;

            case MOVE_LEFT:
                position -= rightDirection * speed;
                break;
        }

        printf("Position: %f %f %f\n", position.x, position.y, position.z);
        printf("Direction: %f %f %f\n", frontDirection.x, frontDirection.y, frontDirection.z);
    }

    float angle(
        glm::vec3 a,
        glm::vec3 b) {
        return glm::degrees(glm::acos(glm::dot(a, b)));
    }

    void Camera::setTransition(const glm::vec3& targetPosition, const glm::vec3& targetDirection, float duration) {
        transitioning             = true;
        transitionStartPosition   = position;
        transitionStartDirection  = frontDirection;
        transitionTargetPosition  = targetPosition;
        transitionTargetDirection = targetDirection;
        transitionDuration        = duration;
        transitionTimer           = 0.0f;
    }

    void Camera::updateTransition(float deltaSec) {
        if (!transitioning)
            return;

        transitionTimer += deltaSec;
        float t = glm::clamp(transitionTimer / transitionDuration, 0.0f, 1.0f);

        // Linear interpolation for position and direction
        position       = glm::mix(transitionStartPosition, transitionTargetPosition, t);
        frontDirection = glm::mix(transitionStartDirection, transitionTargetDirection, t);

        // Check if transition is complete
        if (transitionTimer >= transitionDuration) {
            transitioning = false;
        }
    }

    void Camera::rotate(float pitch, float yaw) {
        glm::mat4 matrix = glm::rotate(glm::mat4(1.0f), glm::radians(-yaw), rightDirection);

        glm::vec3 newFrontDirection = glm::normalize(glm::vec3(matrix * glm::vec4(frontDirection, 1.0f)));

        const auto ang = angle(newFrontDirection, upDirection);
        if (ang > 5.0f && ang < 175.0f)
            frontDirection = newFrontDirection;

        matrix = glm::rotate(glm::mat4(1.0f), glm::radians(-pitch), upDirection);

        frontDirection = glm::normalize(glm::vec3(matrix * glm::vec4(frontDirection, 1.0f)));
        rightDirection = glm::normalize(glm::cross(frontDirection, upDirection));
    }

}  // namespace gps
