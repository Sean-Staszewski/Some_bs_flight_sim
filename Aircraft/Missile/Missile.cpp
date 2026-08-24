#include "Missile.h"

Missile::Missile(std::string name, std::string manufacturer)
    : Aircraft(std::move(name), std::move(manufacturer))
{
}

void Missile::updatePhysics(glm::vec3& position, glm::quat& orientation,
                             glm::vec3& momentum, const glm::vec3& thrust, float dt)
{
    applyGravity(position, orientation, momentum, thrust, dt);
    // thrust/guidance physics land here once the motor burn profile is wired in
}
