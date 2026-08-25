#include "Camera.h"

Camera::Camera(glm::vec3 camPos, float yaw, float pitch, float moveSpeed, float mouseSensitivity,
               glm::vec3 front, glm::vec3 right, glm::vec3 up, float fovd)
    : g_camPos(camPos), g_yaw(yaw), g_pitch(pitch), MOVE_SPEED(moveSpeed),
      MOUSE_SENSITIVITY(mouseSensitivity), g_front(front), g_right(right), g_up(up), g_fovd(fovd) {}