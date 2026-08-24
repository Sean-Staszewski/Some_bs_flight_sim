#include "Aircraft.h"

#include <cmath>

Aircraft::Aircraft(std::string name, std::string manufacturer)
    : name_(std::move(name)), manufacturer_(std::move(manufacturer))
{
}

void Aircraft::applyGravity(glm::vec3& position, glm::quat& /*orientation*/,
                             glm::vec3& momentum, const glm::vec3& /*thrust*/, float dt)
{
    momentum.y -= massKg * kGravity * dt;
    position += (momentum / massKg) * dt;
}

void Aircraft::updateMass()
{
    massKg = emptyMassKg + fuelMassKg + armamentsMassKg + countermeasuresMassKg;
}

void Aircraft::updatePhysics(glm::vec3& position, glm::quat& orientation,
                              glm::vec3& momentum, const glm::vec3& thrust, float dt)
{
    applyGravity(position, orientation, momentum, thrust, dt);
}

float Aircraft::updateTemperature(const std::string& tableName, float xValue, float dt, float timeConstantS)
{
    float targetC = interp1d(temperatureTables.at(tableName), xValue);
    float currentC = kAmbientTempC;
    auto it = temperaturesC.find(tableName);
    if (it != temperaturesC.end()) {
        currentC = it->second;
    }
    currentC += (targetC - currentC) * (1.0f - std::exp(-dt / timeConstantS));
    temperaturesC[tableName] = currentC;
    return currentC;
}

SectorVolume Aircraft::staticSector(const std::string& sectorName, const FovConfig& fov)
{
    SectorVolume v;
    v.name = sectorName;
    v.centerAzimuthDeg = 0.0f;
    v.azimuthWidthDeg = fov.azimuthDeg;
    v.centerElevationDeg = 0.0f;
    v.elevationWidthDeg = fov.elevationDeg;
    v.rangeKm = fov.maxRangeKm;
    return v;
}

std::vector<SectorVolume> Aircraft::radarCoverage(float /*t*/) const
{
    return {staticSector("primary", radarConfig)};
}

std::vector<SectorVolume> Aircraft::irCoverage(float /*t*/) const
{
    std::vector<SectorVolume> result;
    for (const auto& [sensorName, fov] : irSensorConfigs) {
        result.push_back(staticSector(sensorName, fov));
    }
    return result;
}

Aircraft::DetectionResult Aircraft::radarDetect(float t, const glm::vec3& sensorPosition,
                                                 const glm::quat& sensorOrientation, const Aircraft& target,
                                                 const glm::vec3& targetPosition,
                                                 const glm::quat& targetOrientation) const
{
    glm::vec3 delta = targetPosition - sensorPosition;
    float rangeKm = glm::length(delta) / 1000.0f;

    glm::vec3 sensorLocalDelta = glm::conjugate(sensorOrientation) * delta;
    auto [bearingDeg, elevationDeg] = bearingElevationDeg(sensorLocalDelta);

    bool inCoverage = false;
    for (const auto& volume : radarCoverage(t)) {
        if (volume.contains(bearingDeg, elevationDeg, rangeKm)) {
            inCoverage = true;
            break;
        }
    }
    if (!inCoverage) {
        return DetectionResult{};
    }

    glm::vec3 targetLocalDelta = glm::conjugate(targetOrientation) * delta;
    glm::vec3 aspectDir = glm::normalize(targetLocalDelta);

    const SignatureRow& row = nearestSignatureRow(target.rcsSignature, aspectDir);

    DetectionResult result;
    result.hit = true;
    result.values = row.values;
    result.values["range_km"] = rangeKm;
    result.values["aspect_x"] = row.direction.x;
    result.values["aspect_y"] = row.direction.y;
    result.values["aspect_z"] = row.direction.z;
    return result;
}

Aircraft::DetectionResult Aircraft::irDetect(float t, const glm::vec3& sensorPosition,
                                              const glm::quat& sensorOrientation, const Aircraft& target,
                                              const glm::vec3& targetPosition,
                                              const glm::quat& targetOrientation) const
{
    glm::vec3 delta = targetPosition - sensorPosition;
    float rangeKm = glm::length(delta) / 1000.0f;

    glm::vec3 sensorLocalDelta = glm::conjugate(sensorOrientation) * delta;
    auto [bearingDeg, elevationDeg] = bearingElevationDeg(sensorLocalDelta);

    bool inCoverage = false;
    for (const auto& volume : irCoverage(t)) {
        if (volume.contains(bearingDeg, elevationDeg, rangeKm)) {
            inCoverage = true;
            break;
        }
    }
    if (!inCoverage) {
        return DetectionResult{};
    }

    glm::vec3 targetLocalDelta = glm::conjugate(targetOrientation) * delta;
    glm::vec3 aspectDir = glm::normalize(targetLocalDelta);

    float totalMwir = 0.0f;
    float totalLwir = 0.0f;
    for (const auto& [componentName, table] : target.irSignatureTables) {
        const SignatureRow& row = nearestSignatureRow(table, aspectDir);

        const auto& tempTable = target.temperatureTables.at(componentName);
        float referenceTempC = tempTable.front().second;
        for (const auto& point : tempTable) {
            referenceTempC = std::max(referenceTempC, point.second);
        }

        float currentTempC = kAmbientTempC;
        auto it = target.temperaturesC.find(componentName);
        if (it != target.temperaturesC.end()) {
            currentTempC = it->second;
        }

        float scale = irTemperatureScale(currentTempC, referenceTempC);
        totalMwir += row.values.at("ir_mwir_w_per_sr") * scale;
        totalLwir += row.values.at("ir_lwir_w_per_sr") * scale;
    }

    DetectionResult result;
    result.hit = true;
    result.values["ir_mwir_w_per_sr"] = totalMwir;
    result.values["ir_lwir_w_per_sr"] = totalLwir;
    result.values["range_km"] = rangeKm;
    return result;
}

std::unordered_map<std::string, float> Aircraft::perceive(const std::unordered_map<std::string, float>& signature,
                                                            const std::vector<std::string>& fields,
                                                            float sensorStrength, float falloffExponent) const
{
    float rangeKm = signature.at("range_km");
    float divisor = rangeKm <= 0.0f ? 1.0f : std::pow(rangeKm, falloffExponent);
    std::unordered_map<std::string, float> result;
    for (const auto& field : fields) {
        result[field] = signature.at(field) * sensorStrength / divisor;
    }
    return result;
}
