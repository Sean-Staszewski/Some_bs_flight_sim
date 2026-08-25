#include "controls.h"
#include "Camera.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

// Globals defined in main.cpp
extern Camera camera;

extern float g_deltaTime;
extern bool g_dragging;




void processInput(GLFWwindow *window, std::vector<ObjInstance>& /*objInstances*/)
{
    float speed = camera.MOVE_SPEED * g_deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.g_camPos += speed * camera.g_front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.g_camPos -= speed * camera.g_front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.g_camPos -= speed * camera.g_right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.g_camPos += speed * camera.g_right;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.g_camPos += speed * camera.g_up;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
        camera.g_camPos -= speed * camera.g_up;
}

void scroll_callback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset)
{
    camera.g_fovd -= (float)yoffset * 2.0f;
    if (camera.g_fovd < 1.0f) camera.g_fovd = 1.0f;
    if (camera.g_fovd > 90.0f) camera.g_fovd = 90.0f;
}

void mouse_button_callback(GLFWwindow* /*window*/, int button, int action, int /*mods*/)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            g_dragging = true;
        } else if (action == GLFW_RELEASE) {
            g_dragging = false;
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

    if (!g_dragging) return;

    camera.g_yaw   += (float)dx * camera.MOUSE_SENSITIVITY; // divide by 10 to make mouse movement less twitchy for camera
    camera.g_pitch -= (float)dy * camera.MOUSE_SENSITIVITY; // divide by 10 to make mouse movement less twitchy for camera

    if (camera.g_pitch >  89.0f) camera.g_pitch =  89.0f;
    if (camera.g_pitch < -89.0f) camera.g_pitch = -89.0f;
}
