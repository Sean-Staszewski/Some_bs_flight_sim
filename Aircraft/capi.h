#pragma once

// Plain C ABI over the C++ Aircraft sensing/temperature math, for calling from Python via
// ctypes. Exists because this environment's only C++ compiler (MinGW win32-threads) can't
// build pybind11 (it needs std::mutex/std::call_once, unavailable under win32-threads) --
// see CMakeLists.txt / the BUILD_PYTHON_BINDINGS option for the pybind11 path this
// replaces. No exceptions or C++ types cross this boundary.

extern "C" {

typedef void* AircraftHandle;

AircraftHandle aircraft_create_f16();
AircraftHandle aircraft_create_aim120();
void aircraft_destroy(AircraftHandle handle);

// --- mass ---
void aircraft_set_empty_mass_kg(AircraftHandle handle, float value);
void aircraft_set_fuel_mass_kg(AircraftHandle handle, float value);
void aircraft_set_armaments_mass_kg(AircraftHandle handle, float value);
void aircraft_set_countermeasures_mass_kg(AircraftHandle handle, float value);
void aircraft_update_mass(AircraftHandle handle);
float aircraft_get_mass_kg(AircraftHandle handle);

// --- radar / IR sensor config ---
void aircraft_set_radar_config(AircraftHandle handle, float max_range_km, float azimuth_deg, float elevation_deg);
void aircraft_set_ir_sensor_config(AircraftHandle handle, const char* sensor_name, float max_range_km,
                                    float azimuth_deg, float elevation_deg);

// --- temperature tables/state ---
// Loads one component's temperature table: parallel x_values[]/temp_values_c[] arrays,
// `count` entries. x_values are whatever independent variable that component uses
// (throttle_percent, afterburner_stage, mach, ...) -- the column name itself isn't
// needed here, only the values; Aircraft.py resolves the column name once when it
// syncs the table in and doesn't pass it to update_temperature after that.
void aircraft_set_temperature_table(AircraftHandle handle, const char* table_name, const float* x_values,
                                     const float* temp_values_c, int count);
float aircraft_update_temperature(AircraftHandle handle, const char* table_name, float x_value, float dt,
                                   float time_constant_s);
float aircraft_get_temperature_c(AircraftHandle handle, const char* table_name);

// --- signatures ---
// Parallel x[]/y[]/z[]/band_a[]/band_b[] arrays, `count` entries. For RCS, band_a/band_b
// are rcs_xband_m2/rcs_sband_m2; for IR, ir_mwir_w_per_sr/ir_lwir_w_per_sr.
void aircraft_set_rcs_signature(AircraftHandle handle, const float* x, const float* y, const float* z,
                                 const float* rcs_xband_m2, const float* rcs_sband_m2, int count);
void aircraft_set_ir_signature_component(AircraftHandle handle, const char* component_name, const float* x,
                                          const float* y, const float* z, const float* ir_mwir_w_per_sr,
                                          const float* ir_lwir_w_per_sr, int count);

// --- detection ---
// orientation is (w, x, y, z), local -> world, same convention as everywhere else in
// this project. Returns 0/1 in *out_hit; other outputs are only meaningful when hit.
void aircraft_radar_detect(AircraftHandle sensor, float t, const float* sensor_position,
                            const float* sensor_orientation, AircraftHandle target, const float* target_position,
                            const float* target_orientation, int* out_hit, float* out_rcs_xband_m2,
                            float* out_rcs_sband_m2, float* out_range_km);

void aircraft_ir_detect(AircraftHandle sensor, float t, const float* sensor_position,
                         const float* sensor_orientation, AircraftHandle target, const float* target_position,
                         const float* target_orientation, int* out_hit, float* out_ir_mwir_w_per_sr,
                         float* out_ir_lwir_w_per_sr, float* out_range_km);

// --- perceive ---
// falloff_exponent: 4.0 for radar (round-trip radar equation), 2.0 for IR (inverse-square).
float aircraft_perceive(float raw_value, float range_km, float sensor_strength, float falloff_exponent);

} // extern "C"
