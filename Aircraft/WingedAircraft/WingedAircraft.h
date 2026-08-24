#pragma once

#include "../Aircraft.h"

class WingedAircraft : public Aircraft {
public:
    WingedAircraft(std::string name, std::string manufacturer);

    void updatePhysics(glm::vec3& position, glm::quat& orientation,
                        glm::vec3& momentum, const glm::vec3& thrust, float dt) override;
};
