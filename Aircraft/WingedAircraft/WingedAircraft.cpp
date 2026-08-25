#include "WingedAircraft.h"

void WingedAircraft::applyPhysics(float dt) {
    // Apply physics specific to winged aircraft, such as lift and drag
    // For now, we can call the base Aircraft's applyPhysics method
    Aircraft::applyPhysics(dt);
}