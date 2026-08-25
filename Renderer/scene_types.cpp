#include "scene_types.h"
#include "glutils.h"
#include <iostream>
#include <limits>

struct WorldAABB {
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());
};


Scene g_scene;

void ObjInstance::selfTexture() {
}


void initializeScene() {
    g_scene.objects.clear();

    for (auto obj : g_scene.objects) {
        if (!loadOBJ(obj.modelPath, obj.originalVertices)) {
            std::cout << "Failed to load model: " << obj.modelPath << "\n";
        }

    }
}

