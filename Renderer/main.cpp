// template based on material from learnopengl.com
#define GLEW_STATIC
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glutils.h"
#include "scene_types.h"
#include "controls.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Camera.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);


// settings
const unsigned int SCR_WIDTH  = 800;
const unsigned int SCR_HEIGHT = 600;

// left mouse button held -- gates camera-look dragging
bool g_dragging = false;

// Camera state
Camera camera(glm::vec3(0.0f, 1.0f, 10.0f), -90.0f, 0.0f, 15.0f, 0.25f, 
                glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 
                glm::vec3(0.0f, 1.0f, 0.0f), 45.0f);

float g_deltaTime = 0.0f;
float g_lastFrame = 0.0f;


// Selected object index (-1 = none, 0 = first, 1 = second)
int main(int argc, char* argv[])
{

    std::vector<float> objVertices;

    // Initialize scene objects
    initializeScene();

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "viewGL", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glewInit();
    glEnable(GL_DEPTH_TEST);

    std::string phongVert = loadShaderSource("shaders/phong.vs");
    std::string phongFrag = loadShaderSource("shaders/phong.fs");
    std::string wireframeFrag = loadShaderSource("shaders/wireframe.fs");
    if (phongVert.empty() || phongFrag.empty() || wireframeFrag.empty()) {
        std::cerr << "Failed to load shader files.\n";
        return -1;
    }
    unsigned int phongProgram = buildShaderProgram(phongVert.c_str(), phongFrag.c_str());
    unsigned int wireframeProgram = buildShaderProgram(phongVert.c_str(), wireframeFrag.c_str());

    for (auto& inst : g_scene.objects) {
        glGenVertexArrays(1, &inst.VAO);
        glGenBuffers(1, &inst.VBO);
        glBindVertexArray(inst.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, inst.VBO);
        glBufferData(GL_ARRAY_BUFFER,
                     inst.originalVertices.size() * sizeof(float),
                     inst.originalVertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        inst.numVertices = inst.originalVertices.size() / 8;
        
        // Load texture if texturePath is set
        if (!inst.texturePath.empty()) {
            inst.texture = loadTexture(inst.texturePath);
        }
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    float fpsAccum  = 0.0f;
    int   fpsFrames = 0;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        g_deltaTime = currentFrame - g_lastFrame;
        g_lastFrame = currentFrame;

        fpsAccum  += g_deltaTime;
        fpsFrames += 1;
        if (fpsAccum >= 1.0f) {
            std::cout << "[FPS] " << fpsFrames << "\n";
            fpsAccum  -= 1.0f;
            fpsFrames  = 0;
        }

        camera.g_front.x = cos(glm::radians(camera.g_yaw)) * cos(glm::radians(camera.g_pitch));
        camera.g_front.y = sin(glm::radians(camera.g_pitch));
        camera.g_front.z = sin(glm::radians(camera.g_yaw)) * cos(glm::radians(camera.g_pitch));
        camera.g_front   = glm::normalize(camera.g_front);
        camera.g_right   = glm::normalize(glm::cross(camera.g_front, glm::vec3(0.0f, 1.0f, 0.0f)));
        camera.g_up      = glm::normalize(glm::cross(camera.g_right, camera.g_front));

        processInput(window, g_scene.objects);


        glm::mat4 proj = glm::perspective(glm::radians(camera.g_fovd), (float)SCR_WIDTH / (float)SCR_HEIGHT, 1.0f, 50.0f);
        glm::mat4 view = glm::lookAt(camera.g_camPos, camera.g_camPos + camera.g_front, camera.g_up);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto& inst : g_scene.objects) {
            bool useWireframe = (inst.shaderPath == "shaders/wireframe.fs");
            unsigned int shaderProgram = useWireframe ? wireframeProgram : phongProgram;

            glUseProgram(shaderProgram);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, glm::value_ptr(inst.modelMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uView"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uProj"), 1, GL_FALSE, glm::value_ptr(proj));

            if (useWireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glUniform3fv(glGetUniformLocation(shaderProgram, "uWireColor"), 1, glm::value_ptr(inst.color));
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glUniform3f(glGetUniformLocation(shaderProgram, "uLightPos"), 2.0f, 3.0f, 2.0f);
                glUniform3fv(glGetUniformLocation(shaderProgram, "uViewPos"), 1, glm::value_ptr(camera.g_camPos));
                glUniform3f(glGetUniformLocation(shaderProgram, "uLightColor"), 1.0f, 1.0f, 1.0f);
                glUniform1f(glGetUniformLocation(shaderProgram, "uAmbientStrength"), 0.15f);
                glUniform1f(glGetUniformLocation(shaderProgram, "uSpecularStrength"), 0.5f);
                glUniform1f(glGetUniformLocation(shaderProgram, "uShininess"), 32.0f);
                glUniform1i(glGetUniformLocation(shaderProgram, "uTex"), 0);

                // Set this object's base color
                glUniform3fv(glGetUniformLocation(shaderProgram, "uObjectColor"), 1, glm::value_ptr(inst.color));

                // Bind texture if available and inform shader whether to sample it
                if (inst.texture != 0) {
                    glUniform1i(glGetUniformLocation(shaderProgram, "uHasTex"), 1);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, inst.texture);
                } else {
                    glUniform1i(glGetUniformLocation(shaderProgram, "uHasTex"), 0);
                    glBindTexture(GL_TEXTURE_2D, 0);
                }
            }

            glBindVertexArray(inst.VAO);
            glDrawArrays(GL_TRIANGLES, 0, inst.numVertices);
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        static float depthPrintAccum = 0.0f;
        depthPrintAccum += g_deltaTime;

        glFlush();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for (auto& inst : g_scene.objects) {
        glDeleteVertexArrays(1, &inst.VAO);
        glDeleteBuffers(1, &inst.VBO);
        if (inst.texture != 0) {
            glDeleteTextures(1, &inst.texture);
        }
    }
    glDeleteProgram(phongProgram);
    glDeleteProgram(wireframeProgram);
    glfwTerminate();
    return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
