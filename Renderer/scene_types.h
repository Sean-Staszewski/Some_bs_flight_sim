#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-decls"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <GL/glew.h>

struct ObjInstance {
    int index; // Index in the scene's object list

    unsigned int VAO, VBO;
    unsigned int texture = 0;
    unsigned int numVertices;
    std::vector<float> originalVertices;

    // Uniform scale factor
    float scale = 1.0f;
    glm::vec3 color = glm::vec3(0.85f, 0.85f, 0.9f);

    // World-space position
    glm::vec3 position = glm::vec3(0.0f);

    // rotation matrix (driven by mouse drag and R key)
    glm::mat4 localRotation = glm::mat4(1.0f);


    glm::mat4 modelMatrix() const {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
        return T * localRotation * S;
    }

    std::string modelPath;
    std::string texturePath;
    std::string shaderPath = "shaders/phong.fs";

    // Virtual method for objects that generate their own texture
    virtual void selfTexture();

};

struct Scene {
    std::vector<ObjInstance> objects;
};

void initializeScene();

extern Scene g_scene;

#pragma GCC diagnostic pop
