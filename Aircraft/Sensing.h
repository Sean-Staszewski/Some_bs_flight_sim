#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct SectorVolume {
    std::string name;
    float centerAzimuthDeg = 0.0f;
    float azimuthWidthDeg = 0.0f;
    float centerElevationDeg = 0.0f;
    float elevationWidthDeg = 0.0f;
    float rangeKm = 0.0f;

    bool contains(float bearingDeg, float elevationDeg, float rangeKmValue) const;
};

struct SignatureRow {
    glm::vec3 direction{0.0f};
    std::unordered_map<std::string, float> values; // e.g. rcs_xband_m2, ir_mwir_w_per_sr, ...
};

bool withinSector(float bearingDeg, float widthDeg);

// table is a list of (x, y) pairs; interpolates y at xValue, clamped at the ends.
float interp1d(const std::vector<std::pair<float, float>>& table, float xValue);

// Returns the row whose direction is closest (max dot product) to `direction`. table must
// not be empty.
const SignatureRow& nearestSignatureRow(const std::vector<SignatureRow>& table, const glm::vec3& direction);

// Stefan-Boltzmann fourth-power scaling: radiance ~ T_kelvin^4. Scales a value calibrated
// at referenceTempC to what it'd be at currentTempC.
float irTemperatureScale(float currentTempC, float referenceTempC);

// localDir is in body coordinates (local +X = nose, +Y = up, +Z = right), same convention
// as the physics code. Returns {bearingDeg, elevationDeg}.
std::pair<float, float> bearingElevationDeg(const glm::vec3& localDir);
