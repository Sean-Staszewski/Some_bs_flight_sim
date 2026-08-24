#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Sensing.h"

class Aircraft {
public:
    Aircraft(std::string name, std::string manufacturer);
    virtual ~Aircraft() = default;

    // Every physics step shares this signature so subclasses can compose steps
    // (gravity, lift, drag, thrust, ...) uniformly regardless of what each needs.
    // momentum is state (updated in place, like position/orientation); thrust is
    // this frame's commanded thrust vector, an input the step may or may not use.
    virtual void updatePhysics(glm::vec3& position, glm::quat& orientation,
                                glm::vec3& momentum, const glm::vec3& thrust, float dt);

    void applyGravity(glm::vec3& position, glm::quat& orientation,
                       glm::vec3& momentum, const glm::vec3& thrust, float dt);

    float massKg = 0.0f; // total mass; recomputed by updateMass(), don't set directly

    float emptyMassKg = 0.0f;          // structural mass, excludes fuel and loadout
    float fuelMassKg = 0.0f;           // current fuel mass, changes as fuel burns
    float armamentsMassKg = 0.0f;      // total mass of armaments currently on board
    float countermeasuresMassKg = 0.0f; // total mass of countermeasures currently on board

    // Recomputes massKg from the components above. Call after fuel burn, a weapon
    // release, or a countermeasure dispense changes any of them.
    void updateMass();

    const std::string& name() const { return name_; }
    const std::string& manufacturer() const { return manufacturer_; }

    // --- temperature dynamics ---
    // Ported from Aircraft.py's update_temperature/_interp_1d for speed. Python still
    // owns loading data/temperature/*.csv; these tables are populated from Python via
    // the pybind11 bindings, keyed the same way ("engine_core" -> [(throttle_percent,
    // temp_c), ...]).
    std::unordered_map<std::string, std::vector<std::pair<float, float>>> temperatureTables;
    std::unordered_map<std::string, float> temperaturesC; // current per-component temp
    static constexpr float kAmbientTempC = 15.0f;

    // Drifts temperaturesC[tableName] toward temperatureTables[tableName]'s interpolated
    // target at xValue, using the exact exponential solution (correct at any dt). Starts
    // at kAmbientTempC on first call for that component. Returns the new current temp.
    float updateTemperature(const std::string& tableName, float xValue, float dt, float timeConstantS = 10.0f);

    // --- signatures ---
    std::vector<SignatureRow> rcsSignature;
    std::unordered_map<std::string, std::vector<SignatureRow>> irSignatureTables; // one per heat-emitting component

    // --- sensor coverage config (from radar.json / ir_sensor.json) ---
    struct FovConfig {
        float maxRangeKm = 0.0f;
        float azimuthDeg = 0.0f;
        float elevationDeg = 0.0f;
    };
    FovConfig radarConfig;
    std::unordered_map<std::string, FovConfig> irSensorConfigs; // e.g. "primary", "early_warning"

    // Default: static sector(s) centered on the nose. Override per type for
    // sweeping/scanning radars (see F16::radarCoverage).
    virtual std::vector<SectorVolume> radarCoverage(float t) const;
    virtual std::vector<SectorVolume> irCoverage(float t) const;

    struct DetectionResult {
        bool hit = false;
        std::unordered_map<std::string, float> values; // signature fields + "range_km"
    };

    // Bounding-box-first: checks radarCoverage(t)/irCoverage(t) before doing any
    // signature lookup. See Aircraft.py's radar_detect/ir_detect for the full rationale.
    DetectionResult radarDetect(float t, const glm::vec3& sensorPosition, const glm::quat& sensorOrientation,
                                 const Aircraft& target, const glm::vec3& targetPosition,
                                 const glm::quat& targetOrientation) const;
    DetectionResult irDetect(float t, const glm::vec3& sensorPosition, const glm::quat& sensorOrientation,
                              const Aircraft& target, const glm::vec3& targetPosition,
                              const glm::quat& targetOrientation) const;

    // Converts a raw signature (radarDetect's/irDetect's result, which includes
    // "range_km") into what this sensor actually perceives: range dropoff (divided by
    // range_km ** falloffExponent) combined with sensorStrength. falloffExponent=4 for
    // radar (round-trip radar equation), =2 (the default) for IR (one-way, inverse-square).
    std::unordered_map<std::string, float> perceive(const std::unordered_map<std::string, float>& signature,
                                                      const std::vector<std::string>& fields,
                                                      float sensorStrength = 1.0f,
                                                      float falloffExponent = 2.0f) const;

protected:
    std::string name_;
    std::string manufacturer_;

    static constexpr float kGravity = 9.81f; // m/s^2, world +Y is up

    static SectorVolume staticSector(const std::string& sectorName, const FovConfig& fov);
};
