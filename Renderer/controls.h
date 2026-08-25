#pragma once

#include <vector>
#include "scene_types.h"
#include <GLFW/glfw3.h>

void processInput(GLFWwindow *window, std::vector<ObjInstance>& objInstances);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

