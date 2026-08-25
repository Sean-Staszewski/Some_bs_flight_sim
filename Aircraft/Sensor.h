#pragma once
#include "Aircraft.h"
#include <cmath>
#include <glm/glm.hpp>
#include <string>
#include <map>

using namespace std;

class Sensor {
    // Sensor properties and methods would be defined here
    string type;

    float range;
    float strength;
    float horizontalFOV; // if 360 then it's a full-sphere sensor
    float verticalFOV; // if 360 then it's a full-sphere sensor

    float updateFrequency; // in Hz, how often the sensor updates its readings
    float sweepingAngle; // in degrees, for sensors that sweep (like radar)
    float sweepSpeed; // in degrees per second, for sensors that sweep (like radar)
    float maxSweepingAngle;
    bool sweepLeft; // 1 for clockwise, -1 for counter-clockwise

    bool isActive;

    void sweep(float dt); // Update the sensor's sweep based on time delta
};

map<Aircraft, map<string, float>> sense(const glm::vec3& pos1, glm::mat4 orientation, float horizontalFOV, 
            float verticalFOV, float range, string type, vector<Aircraft> aircrafts);

map<string, float> getSignature(glm::vec3 direction, string type, const Aircraft& aircraft);