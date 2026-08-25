#include "F_16.h"

F_16::F_16(glm::vec3 initialPosition, glm::mat4 initialOrientation) {
    setPosition(initialPosition);
    setLocalRotation(initialOrientation);
    passToRenderer();
    objInstance.modelPath = "../Aircraft/WingedAircraft/F-16/model/F_16.obj";
    // Raw mesh spans roughly 115x94x25 units (x/y/z) -- shrink it to a scene-appropriate size.
    objInstance.scale = 0.05f;
}

F_16::~F_16() {
}