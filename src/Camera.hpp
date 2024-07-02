#ifndef Camera_hpp
#define Camera_hpp

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace gps {

    enum MOVE_DIRECTION { MOVE_FORWARD,
                          MOVE_BACKWARD,
                          MOVE_RIGHT,
                          MOVE_LEFT };

    // https://learnopengl.com/Getting-started/Camera
    class Camera {
       public:
        // Camera constructor
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);
        // return the view matrix, using the glm::lookAt() function
        glm::mat4 getViewMatrix();
        // update the camera internal parameters following a camera move event
        void move(MOVE_DIRECTION direction, float speed);
        // update the camera internal parameters following a camera rotate event
        // yaw - camera rotation around the y axis
        // pitch - camera rotation around the x axis
        void rotate(float pitch, float yaw);
        void setTransition(const glm::vec3& targetPosition, const glm::vec3& targetDirection, float duration);
        void updateTransition(float deltaSec);

        glm::vec3 position;
        bool transitioning;

       private:
        glm::vec3 frontDirection;
        glm::vec3 upDirection;
        glm::vec3 rightDirection;

        glm::vec3 transitionStartPosition;
        glm::vec3 transitionStartDirection;
        glm::vec3 transitionTargetPosition;
        glm::vec3 transitionTargetDirection;
        float transitionDuration;
        float transitionTimer;
    };
}  // namespace gps

#endif /* Camera_hpp */
