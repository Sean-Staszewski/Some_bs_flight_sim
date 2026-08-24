#include <array>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Aircraft.h"
#include "Missile/Missile.h"
#include "Missile/AIM_120/Aim120.h"
#include "Sensing.h"
#include "WingedAircraft/F-16/F16.h"
#include "WingedAircraft/WingedAircraft.h"

namespace py = pybind11;

namespace {

glm::vec3 vec3FromTuple(const std::array<float, 3>& t)
{
    return glm::vec3(t[0], t[1], t[2]);
}

glm::quat quatFromTuple(const std::array<float, 4>& t)
{
    return glm::quat(t[0], t[1], t[2], t[3]); // (w, x, y, z)
}

// Converts data/rcs_signature.csv's already-loaded shape (list of {"x":,"y":,"z":,
// "rcs_xband_m2":, ...}) into a std::vector<SignatureRow>, same shape Python already
// produces via Aircraft._load_table_csv -- no new parsing, just a type conversion.
std::vector<SignatureRow> signatureTableFromRows(const std::vector<std::unordered_map<std::string, float>>& rows)
{
    std::vector<SignatureRow> table;
    table.reserve(rows.size());
    for (const auto& row : rows) {
        SignatureRow r;
        r.direction = glm::vec3(row.at("x"), row.at("y"), row.at("z"));
        for (const auto& [key, value] : row) {
            if (key != "x" && key != "y" && key != "z") {
                r.values[key] = value;
            }
        }
        table.push_back(std::move(r));
    }
    return table;
}

// Converts data/temperature/<name>.csv's already-loaded shape (list of {"<x_key>":,
// "temp_c":}) into (x, temp_c) pairs -- the x_key column name doesn't matter here, only
// that there are exactly two columns and one of them is "temp_c".
std::vector<std::pair<float, float>> temperatureTableFromRows(
    const std::vector<std::unordered_map<std::string, float>>& rows)
{
    std::vector<std::pair<float, float>> table;
    table.reserve(rows.size());
    for (const auto& row : rows) {
        float tempC = row.at("temp_c");
        float xValue = 0.0f;
        for (const auto& [key, value] : row) {
            if (key != "temp_c") {
                xValue = value;
                break;
            }
        }
        table.push_back({xValue, tempC});
    }
    return table;
}

py::object detectionResultToPy(const Aircraft::DetectionResult& result)
{
    if (!result.hit) {
        return py::none();
    }
    py::dict d;
    for (const auto& [key, value] : result.values) {
        d[py::str(key)] = value;
    }
    return d;
}

} // namespace

PYBIND11_MODULE(aircraft_cpp, m)
{
    m.doc() = "C++ port of Aircraft.py's temperature dynamics and sensing math, for speed.";

    py::class_<SectorVolume>(m, "SectorVolume")
        .def(py::init<>())
        .def_readwrite("name", &SectorVolume::name)
        .def_readwrite("center_azimuth_deg", &SectorVolume::centerAzimuthDeg)
        .def_readwrite("azimuth_width_deg", &SectorVolume::azimuthWidthDeg)
        .def_readwrite("center_elevation_deg", &SectorVolume::centerElevationDeg)
        .def_readwrite("elevation_width_deg", &SectorVolume::elevationWidthDeg)
        .def_readwrite("range_km", &SectorVolume::rangeKm)
        .def("contains", &SectorVolume::contains);

    py::class_<Aircraft::FovConfig>(m, "FovConfig")
        .def(py::init<>())
        .def_readwrite("max_range_km", &Aircraft::FovConfig::maxRangeKm)
        .def_readwrite("azimuth_deg", &Aircraft::FovConfig::azimuthDeg)
        .def_readwrite("elevation_deg", &Aircraft::FovConfig::elevationDeg);

    py::class_<Aircraft>(m, "Aircraft")
        .def_readwrite("mass_kg", &Aircraft::massKg)
        .def_readwrite("empty_mass_kg", &Aircraft::emptyMassKg)
        .def_readwrite("fuel_mass_kg", &Aircraft::fuelMassKg)
        .def_readwrite("armaments_mass_kg", &Aircraft::armamentsMassKg)
        .def_readwrite("countermeasures_mass_kg", &Aircraft::countermeasuresMassKg)
        .def("update_mass", &Aircraft::updateMass)
        .def_readwrite("temperatures_c", &Aircraft::temperaturesC)
        .def_readwrite("radar_config", &Aircraft::radarConfig)
        .def_readwrite("ir_sensor_configs", &Aircraft::irSensorConfigs)
        .def(
            "set_temperature_tables",
            [](Aircraft& self, const std::unordered_map<std::string, std::vector<std::unordered_map<std::string, float>>>& tables) {
                std::unordered_map<std::string, std::vector<std::pair<float, float>>> converted;
                for (const auto& [name, rows] : tables) {
                    converted[name] = temperatureTableFromRows(rows);
                }
                self.temperatureTables = std::move(converted);
            },
            "Takes Python's already-loaded temperature_tables dict (name -> list of row dicts).")
        .def(
            "set_rcs_signature",
            [](Aircraft& self, const std::vector<std::unordered_map<std::string, float>>& rows) {
                self.rcsSignature = signatureTableFromRows(rows);
            },
            "Takes Python's already-loaded rcs_signature list (of {x,y,z,rcs_*} row dicts).")
        .def(
            "set_ir_signature_tables",
            [](Aircraft& self, const std::unordered_map<std::string, std::vector<std::unordered_map<std::string, float>>>& tables) {
                std::unordered_map<std::string, std::vector<SignatureRow>> converted;
                for (const auto& [name, rows] : tables) {
                    converted[name] = signatureTableFromRows(rows);
                }
                self.irSignatureTables = std::move(converted);
            },
            "Takes Python's already-loaded ir_signature_tables dict (name -> list of row dicts).")
        .def("update_temperature", &Aircraft::updateTemperature, py::arg("table_name"), py::arg("x_value"),
             py::arg("dt"), py::arg("time_constant_s") = 10.0f)
        .def(
            "radar_coverage",
            [](const Aircraft& self, float t) { return self.radarCoverage(t); })
        .def(
            "ir_coverage",
            [](const Aircraft& self, float t) { return self.irCoverage(t); })
        .def(
            "radar_detect",
            [](const Aircraft& self, float t, std::array<float, 3> sensorPos, std::array<float, 4> sensorOrient,
               const Aircraft& target, std::array<float, 3> targetPos, std::array<float, 4> targetOrient) {
                auto result = self.radarDetect(t, vec3FromTuple(sensorPos), quatFromTuple(sensorOrient), target,
                                                vec3FromTuple(targetPos), quatFromTuple(targetOrient));
                return detectionResultToPy(result);
            },
            py::arg("t"), py::arg("sensor_position"), py::arg("sensor_orientation"), py::arg("target"),
            py::arg("target_position"), py::arg("target_orientation"))
        .def(
            "ir_detect",
            [](const Aircraft& self, float t, std::array<float, 3> sensorPos, std::array<float, 4> sensorOrient,
               const Aircraft& target, std::array<float, 3> targetPos, std::array<float, 4> targetOrient) {
                auto result = self.irDetect(t, vec3FromTuple(sensorPos), quatFromTuple(sensorOrient), target,
                                             vec3FromTuple(targetPos), quatFromTuple(targetOrient));
                return detectionResultToPy(result);
            },
            py::arg("t"), py::arg("sensor_position"), py::arg("sensor_orientation"), py::arg("target"),
            py::arg("target_position"), py::arg("target_orientation"))
        .def("perceive", &Aircraft::perceive, py::arg("signature"), py::arg("fields"),
             py::arg("sensor_strength") = 1.0f, py::arg("falloff_exponent") = 2.0f);

    py::class_<WingedAircraft, Aircraft>(m, "WingedAircraft");

    py::class_<F16, WingedAircraft>(m, "F16")
        .def(py::init<>())
        .def_readwrite("radar_sweep_period_s", &F16::radarSweepPeriodS)
        .def_readwrite("radar_beam_width_deg", &F16::radarBeamWidthDeg);

    py::class_<Missile, Aircraft>(m, "Missile");

    py::class_<Aim120, Missile>(m, "Aim120").def(py::init<>());
}
