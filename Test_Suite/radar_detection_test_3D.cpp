#include "radar_detection_test_3D.h"
#include <iostream>


CameraWithRadar::CameraWithRadar(glm::vec3 camPos) : Camera(camPos) {
    radar = Sensor();

    radar.type = "radar"; // Set sensor type to radar

    radar.range = 50.0f; // Set radar range to 50 units
    radar.horizontalFOV = 60.0f; // Set horizontal FOV to 60 degrees
    radar.verticalFOV = 60.0f; // Set vertical FOV to 60 degrees

    radar.updateFrequency = 1.0f; // Update radar every second
    radar.sweepingAngle = 0.0f; // Start sweeping from 0 degrees
    radar.sweepSpeed = 0.0f; // Sweep at 30 degrees per second
    radar.maxSweepingAngle = 0; // Full sweep
    radar.sweepLeft = true; // Sweep left (clockwise)

    radar.isActive = true; // Activate the radar

}

void CameraWithRadar::printRadarValues(unordered_map<const Aircraft*, unordered_map<string, float>>& radarResults) {
    std::cout << "Radar Detection Results:\n";
    for (const auto& entry : radarResults) {
        const Aircraft* aircraft = entry.first;
        const auto& signature = entry.second;

        std::cout << "Aircraft: " << aircraft->getName() << "\n";
        for (const auto& sigEntry : signature) {
            std::cout << "  " << sigEntry.first << ": " << sigEntry.second << "\n";
        }
    }
}

