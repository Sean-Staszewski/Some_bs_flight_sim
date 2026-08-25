#include "controls.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

// Globals defined in main.cpp
extern glm::vec3   g_camPos;
extern float       g_yaw;
extern float       g_pitch;
extern float       MOVE_SPEED;
extern float       MOUSE_SENSITIVITY;

extern glm::vec3 g_front;
extern glm::vec3 g_right;
extern glm::vec3 g_up;

extern float g_deltaTime;
extern bool g_mouselocked;
extern float g_fov;

void processInput(GLFWwindow *window, std::vector<ObjInstance>& /*objInstances*/)
{
    float speed = MOVE_SPEED * g_deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) g_camPos += speed * g_front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) g_camPos -= speed * g_front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) g_camPos -= speed * g_right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) g_camPos += speed * g_right;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        g_camPos += speed * g_up;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
        g_camPos -= speed * g_up;
}

void scroll_callback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset)
{
    g_fov -= (float)yoffset * 2.0f;
    if (g_fov < 1.0f) g_fov = 1.0f;
    if (g_fov > 90.0f) g_fov = 90.0f;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int /*mods*/)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            g_mouselocked = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        } else if (action == GLFW_RELEASE) {
            g_mouselocked = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void mouse_callback(GLFWwindow* /*window*/, double xpos, double ypos)
{
    static double lastX = xpos;
    static double lastY = ypos;

    double dx = xpos - lastX;
    double dy = ypos - lastY;

    lastX = xpos;
    lastY = ypos;

    g_yaw   += (float)dx * MOUSE_SENSITIVITY; // divide by 10 to make mouse movement less twitchy for camera
    g_pitch -= (float)dy * MOUSE_SENSITIVITY; // divide by 10 to make mouse movement less twitchy for camera

    if (g_pitch >  89.0f) g_pitch =  89.0f;
    if (g_pitch < -89.0f) g_pitch = -89.0f;
}
