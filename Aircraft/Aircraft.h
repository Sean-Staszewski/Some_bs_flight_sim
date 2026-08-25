#include "Sensor.h"
#include <vector>
#include "../Renderer/scene_types.h"


using namespace std;


class Aircraft {

    ObjInstance objInstance; // ObjInstance representing the aircraft
    Sensor vector<Sensor> sensors; // sensor array/vector
    Physics physics; // struct for physics state

    string name;
    string path;

    glm::vec3 getPosition() {
        return physics.position;
    }

    glm::mat4 getLocalRotation() {
        return physics.orientation; // Assuming orientation is stored as a quaternion
    }

    glm::vec3 getVelocity() {
        // Placeholder for velocity calculation
        return physics.velocity; // Assume stationary for now
    }

    void setLocalRotation(glm::mat4 rotation) {
        physics.orientation = rotation;
    }

    void setPosition(glm::vec3 newPos) {
        physics.position = newPos;
    }

    void passToRenderer() {
        objInstance.position = physics.position;
        objInstance.localRotation = physics.orientation; // Assuming orientation is stored as a quaternion
    }

    void applyPhysics(float dt) {

        physics.velocity += physics.acceleration * dt;
        physics.position += physics.velocity * dt;  
        physics.acceleration = glm::vec3(0.0f, -9.81f, 0.0f) * dt; // Gravity
    };
};

