#include "Sensing.h"

#include <algorithm>
#include <cmath>

bool SectorVolume::contains(float bearingDeg, float elevationDeg, float rangeKmValue) const
{
    bool azOk = withinSector(bearingDeg - centerAzimuthDeg, azimuthWidthDeg);
    bool elOk = withinSector(elevationDeg - centerElevationDeg, elevationWidthDeg);
    bool rangeOk = rangeKmValue <= rangeKm;
    return azOk && elOk && rangeOk;
}

bool withinSector(float bearingDeg, float widthDeg)
{
    float wrapped = std::fmod(bearingDeg + 180.0f, 360.0f);
    if (wrapped < 0.0f) {
        wrapped += 360.0f;
    }
    wrapped -= 180.0f;
    return std::abs(wrapped) <= widthDeg / 2.0f;
}

float interp1d(const std::vector<std::pair<float, float>>& table, float xValue)
{
    std::vector<std::pair<float, float>> rows = table;
    std::sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

    if (xValue <= rows.front().first) {
        return rows.front().second;
    }
    if (xValue >= rows.back().first) {
        return rows.back().second;
    }
    for (size_t i = 0; i + 1 < rows.size(); ++i) {
        const auto& lo = rows[i];
        const auto& hi = rows[i + 1];
        if (xValue >= lo.first && xValue <= hi.first) {
            float span = hi.first - lo.first;
            float frac = span == 0.0f ? 0.0f : (xValue - lo.first) / span;
            return lo.second + frac * (hi.second - lo.second);
        }
    }
    return rows.back().second; // unreachable given the clamps above
}

const SignatureRow& nearestSignatureRow(const std::vector<SignatureRow>& table, const glm::vec3& direction)
{
    const SignatureRow* best = &table.front();
    float bestDot = glm::dot(best->direction, direction);
    for (const auto& row : table) {
        float d = glm::dot(row.direction, direction);
        if (d > bestDot) {
            bestDot = d;
            best = &row;
        }
    }
    return *best;
}

float irTemperatureScale(float currentTempC, float referenceTempC)
{
    float currentK = currentTempC + 273.15f;
    float referenceK = referenceTempC + 273.15f;
    float ratio = currentK / referenceK;
    return ratio * ratio * ratio * ratio;
}

std::pair<float, float> bearingElevationDeg(const glm::vec3& localDir)
{
    float bearingDeg = glm::degrees(std::atan2(localDir.z, localDir.x));
    float horizontalDist = std::sqrt(localDir.x * localDir.x + localDir.z * localDir.z);
    float elevationDeg = glm::degrees(std::atan2(localDir.y, horizontalDist));
    return {bearingDeg, elevationDeg};
}
