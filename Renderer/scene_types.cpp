#include "scene_types.h"
#include "glutils.h"
#include <iostream>
#include <limits>
#include "../Aircraft/WingedAircraft/F-16/F_16.h"

struct WorldAABB {
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());
};


Scene g_scene;

F_16 f16(glm::vec3(0.0f, 1.0f, 0.0f), glm::mat4(1.0f));

void initializeScene() {
    g_scene.objects.clear();
    g_scene.objects.push_back(f16.objInstance);

    for (auto& obj : g_scene.objects) {
        if (!loadOBJ(obj.modelPath, obj.originalVertices)) {
            std::cout << "Failed to load model: " << obj.modelPath << "\n";
        }

    }
}

