#include "WingedAircraft.h"

WingedAircraft::WingedAircraft(std::string name, std::string manufacturer)
    : Aircraft(std::move(name), std::move(manufacturer))
{
}

void WingedAircraft::updatePhysics(glm::vec3& position, glm::quat& orientation,
                                    glm::vec3& momentum, const glm::vec3& thrust, float dt)
{
    applyGravity(position, orientation, momentum, thrust, dt);
    // lift/drag/thrust land here once wing physics stats (physics.json) are wired in
}
