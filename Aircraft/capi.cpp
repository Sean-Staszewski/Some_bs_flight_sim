#include "capi.h"

#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include "Aircraft.h"
#include "Missile/AIM_120/Aim120.h"
#include "Sensing.h"
#include "WingedAircraft/F-16/F16.h"

namespace {

Aircraft* asAircraft(AircraftHandle handle)
{
    return static_cast<Aircraft*>(handle);
}

glm::vec3 toVec3(const float* p)
{
    return glm::vec3(p[0], p[1], p[2]);
}

glm::quat toQuat(const float* p)
{
    return glm::quat(p[0], p[1], p[2], p[3]); // (w, x, y, z)
}

std::vector<SignatureRow> buildSignatureTable(const float* x, const float* y, const float* z, const float* bandA,
                                               const float* bandB, int count, const char* bandAName,
                                               const char* bandBName)
{
    std::vector<SignatureRow> table;
    table.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        SignatureRow row;
        row.direction = glm::vec3(x[i], y[i], z[i]);
        row.values[bandAName] = bandA[i];
        row.values[bandBName] = bandB[i];
        table.push_back(std::move(row));
    }
    return table;
}

} // namespace

extern "C" {

AircraftHandle aircraft_create_f16()
{
    return new F16();
}

AircraftHandle aircraft_create_aim120()
{
    return new Aim120();
}

void aircraft_destroy(AircraftHandle handle)
{
    delete asAircraft(handle);
}

void aircraft_set_empty_mass_kg(AircraftHandle handle, float value)
{
    asAircraft(handle)->emptyMassKg = value;
}

void aircraft_set_fuel_mass_kg(AircraftHandle handle, float value)
{
    asAircraft(handle)->fuelMassKg = value;
}

void aircraft_set_armaments_mass_kg(AircraftHandle handle, float value)
{
    asAircraft(handle)->armamentsMassKg = value;
}

void aircraft_set_countermeasures_mass_kg(AircraftHandle handle, float value)
{
    asAircraft(handle)->countermeasuresMassKg = value;
}

void aircraft_update_mass(AircraftHandle handle)
{
    asAircraft(handle)->updateMass();
}

float aircraft_get_mass_kg(AircraftHandle handle)
{
    return asAircraft(handle)->massKg;
}

void aircraft_set_radar_config(AircraftHandle handle, float max_range_km, float azimuth_deg, float elevation_deg)
{
    Aircraft::FovConfig cfg;
    cfg.maxRangeKm = max_range_km;
    cfg.azimuthDeg = azimuth_deg;
    cfg.elevationDeg = elevation_deg;
    asAircraft(handle)->radarConfig = cfg;
}

void aircraft_set_ir_sensor_config(AircraftHandle handle, const char* sensor_name, float max_range_km,
                                    float azimuth_deg, float elevation_deg)
{
    Aircraft::FovConfig cfg;
    cfg.maxRangeKm = max_range_km;
    cfg.azimuthDeg = azimuth_deg;
    cfg.elevationDeg = elevation_deg;
    asAircraft(handle)->irSensorConfigs[sensor_name] = cfg;
}

void aircraft_set_temperature_table(AircraftHandle handle, const char* table_name, const float* x_values,
                                     const float* temp_values_c, int count)
{
    std::vector<std::pair<float, float>> table;
    table.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        table.push_back({x_values[i], temp_values_c[i]});
    }
    asAircraft(handle)->temperatureTables[table_name] = std::move(table);
}

float aircraft_update_temperature(AircraftHandle handle, const char* table_name, float x_value, float dt,
                                   float time_constant_s)
{
    return asAircraft(handle)->updateTemperature(table_name, x_value, dt, time_constant_s);
}

float aircraft_get_temperature_c(AircraftHandle handle, const char* table_name)
{
    Aircraft* a = asAircraft(handle);
    auto it = a->temperaturesC.find(table_name);
    return it != a->temperaturesC.end() ? it->second : Aircraft::kAmbientTempC;
}

void aircraft_set_rcs_signature(AircraftHandle handle, const float* x, const float* y, const float* z,
                                 const float* rcs_xband_m2, const float* rcs_sband_m2, int count)
{
    asAircraft(handle)->rcsSignature =
        buildSignatureTable(x, y, z, rcs_xband_m2, rcs_sband_m2, count, "rcs_xband_m2", "rcs_sband_m2");
}

void aircraft_set_ir_signature_component(AircraftHandle handle, const char* component_name, const float* x,
                                          const float* y, const float* z, const float* ir_mwir_w_per_sr,
                                          const float* ir_lwir_w_per_sr, int count)
{
    asAircraft(handle)->irSignatureTables[component_name] = buildSignatureTable(
        x, y, z, ir_mwir_w_per_sr, ir_lwir_w_per_sr, count, "ir_mwir_w_per_sr", "ir_lwir_w_per_sr");
}

void aircraft_radar_detect(AircraftHandle sensor, float t, const float* sensor_position,
                            const float* sensor_orientation, AircraftHandle target, const float* target_position,
                            const float* target_orientation, int* out_hit, float* out_rcs_xband_m2,
                            float* out_rcs_sband_m2, float* out_range_km)
{
    auto result = asAircraft(sensor)->radarDetect(t, toVec3(sensor_position), toQuat(sensor_orientation),
                                                   *asAircraft(target), toVec3(target_position),
                                                   toQuat(target_orientation));
    *out_hit = result.hit ? 1 : 0;
    if (result.hit) {
        *out_rcs_xband_m2 = result.values.at("rcs_xband_m2");
        *out_rcs_sband_m2 = result.values.at("rcs_sband_m2");
        *out_range_km = result.values.at("range_km");
    }
}

void aircraft_ir_detect(AircraftHandle sensor, float t, const float* sensor_position,
                         const float* sensor_orientation, AircraftHandle target, const float* target_position,
                         const float* target_orientation, int* out_hit, float* out_ir_mwir_w_per_sr,
                         float* out_ir_lwir_w_per_sr, float* out_range_km)
{
    auto result = asAircraft(sensor)->irDetect(t, toVec3(sensor_position), toQuat(sensor_orientation),
                                                *asAircraft(target), toVec3(target_position),
                                                toQuat(target_orientation));
    *out_hit = result.hit ? 1 : 0;
    if (result.hit) {
        *out_ir_mwir_w_per_sr = result.values.at("ir_mwir_w_per_sr");
        *out_ir_lwir_w_per_sr = result.values.at("ir_lwir_w_per_sr");
        *out_range_km = result.values.at("range_km");
    }
}

float aircraft_perceive(float raw_value, float range_km, float sensor_strength, float falloff_exponent)
{
    float divisor = range_km <= 0.0f ? 1.0f : std::pow(range_km, falloff_exponent);
    return raw_value * sensor_strength / divisor;
}

} // extern "C"
