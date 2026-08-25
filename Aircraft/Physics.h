#include <glm/glm.hpp>

struct Physics {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec3 angularVelocity; // in radians per second
    glm::vec3 angularAcceleration; // in radians per second squared
    glm::quat orientation; // Quaternion representing the orientation of the aircraft
    float mass; // in kilograms
    glm::vec3 thrust; // in newtons
}