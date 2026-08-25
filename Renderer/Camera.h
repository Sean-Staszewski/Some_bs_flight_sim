#include <glm/glm.hpp>

struct Camera {

    // Camera state
    glm::vec3 g_camPos;
    float     g_yaw;   // start facing -Z
    float     g_pitch;
    float     MOVE_SPEED;     // units per second
    float     MOUSE_SENSITIVITY;
    
    glm::vec3 g_front;
    glm::vec3 g_right;
    glm::vec3 g_up;

    float g_fovd;

    Camera(glm::vec3 camPos = glm::vec3(0.0f, 1.0f, 10.0f),
           float yaw = -90.0f,
           float pitch = 0.0f,
           float moveSpeed = 15.0f,
           float mouseSensitivity = 0.25f,
           glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f),
           glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float fovd = 45.0f);

    Camera(const Camera&);

    Camera& operator=(const Camera&);
};