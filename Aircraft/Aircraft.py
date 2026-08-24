import csv
import inspect
import json
from pathlib import Path

from aircraft_cpp import CppAircraft
from Armaments.Armament import Armament, load_armament_type
from Countermeasures.Countermeasure import Countermeasure, load_countermeasure_type

DATA_ROOT = Path(__file__).resolve().parent


class Aircraft:
    def __init__(self, name, manufacturer):

        self.name = name
        self.manufacturer = manufacturer
        self.path = Path(inspect.getfile(type(self))).resolve().parent
        self.data_path = self.path / "data"

        self.radar = self._load_json("radar.json")
        self.ir_sensor = self._load_json("ir_sensor.json")
        self.radio = self._load_json("radio.json")

        self.rcs_signature = self._load_table_csv(self.data_path / "rcs_signature.csv")
        self.ir_signature_tables = self._load_table_dir("ir_signature")

        self.fuel_tables = self._load_table_dir("fuel")
        self.temperature_tables = self._load_table_dir("temperature")
        self.temperatures_c = {}  # mirrors the C++ backend's state, for external inspection

        self.armaments: dict[Armament, int] = self._load_loadout("armaments", load_armament_type)
        self.countermeasures: dict[Countermeasure, int] = self._load_loadout("countermeasures", load_countermeasure_type)

        # Temperature dynamics and sensing math (interpolation, sector containment,
        # aspect matching, Stefan-Boltzmann scaling, range falloff) all live in C++ now
        # for speed -- see Aircraft/capi.h and aircraft_cpp.py. Python still owns all the
        # file I/O above; this just mirrors the parsed data into the C++ backend.
        self._cpp = self._create_cpp_backend()
        self._sync_cpp_backend()

    def _create_cpp_backend(self):
        """Every concrete aircraft type must override this to return its matching
        aircraft_cpp.CppAircraft (e.g. CppAircraft.f16()) -- see F16.py/Aim120.py."""
        raise NotImplementedError(f"{type(self).__name__} has no C++ backend wired up (see aircraft_cpp.py)")

    def _sync_cpp_backend(self):
        fov = self.radar["field_of_regard_deg"]
        self._cpp.set_radar_config(self.radar["max_range_km"], fov["azimuth"], fov["elevation"])

        for sensor_name, cfg in self.ir_sensor.items():
            ir_fov = cfg["field_of_view_deg"]
            self._cpp.set_ir_sensor_config(sensor_name, cfg["max_range_km"], ir_fov["azimuth"], ir_fov["elevation"])

        for table_name, rows in self.temperature_tables.items():
            x_key = next(k for k in rows[0] if k != "temp_c")
            self._cpp.set_temperature_table(table_name, rows, x_key)

        self._cpp.set_rcs_signature(self.rcs_signature)
        for component_name, rows in self.ir_signature_tables.items():
            self._cpp.set_ir_signature_component(component_name, rows)

    def _load_json(self, filename):
        with open(self.data_path / filename) as f:
            return json.load(f)

    def _load_table_dir(self, dirname):
        """One entry per CSV in data/<dirname>/, keyed by filename (e.g. data/fuel/fuel_to_thrust.csv -> "fuel_to_thrust")."""
        table_dir = self.data_path / dirname
        if not table_dir.is_dir():
            return {}
        return {
            csv_file.stem: self._load_table_csv(csv_file)
            for csv_file in sorted(table_dir.glob("*.csv"))
        }

    def _load_table_csv(self, path):
        with open(path, newline="") as f:
            reader = csv.DictReader(f)
            return [{k: _try_float(v) for k, v in row.items()} for row in reader]

    def _load_loadout(self, key, type_loader):
        """Reads data/loadout.json's "armaments"/"countermeasures" list (type name + count) into
        {instance: count}, resolving each type name to its class via type_loader."""
        loadout_file = self.data_path / "loadout.json"
        if not loadout_file.is_file():
            return {}
        with open(loadout_file) as f:
            loadout = json.load(f)
        return {
            type_loader(entry["type"])(): entry["count"]
            for entry in loadout.get(key, [])
        }

    def update_temperature(self, table_name, x_value, dt, time_constant_s=10.0):
        """Drifts this component's temperature toward data/temperature/<table_name>.csv's
        interpolated target at x_value (e.g. update_temperature("engine_core", 80, dt)),
        via the C++ backend's exact exponential heating/cooling curve. Returns (and mirrors
        into self.temperatures_c) the new current temperature.
        """
        current_c = self._cpp.update_temperature(table_name, x_value, dt, time_constant_s)
        self.temperatures_c[table_name] = current_c
        return current_c

    def radar_detect(self, t, sensor_position, sensor_orientation,
                      target, target_position, target_orientation):
        """Radar detection against `target` (another Aircraft/Missile). position is global
        XYZ in meters; orientation is a (w, x, y, z) quaternion, local -> world, same
        convention as the C++ physics code. See Aircraft::radarDetect in Aircraft.cpp for
        the full rationale (bounding-box-first, then nearest-aspect RCS lookup).
        """
        return self._cpp.radar_detect(t, sensor_position, sensor_orientation,
                                       target._cpp, target_position, target_orientation)

    def ir_detect(self, t, sensor_position, sensor_orientation,
                  target, target_position, target_orientation):
        """IR detection against `target`. See Aircraft::irDetect in Aircraft.cpp for the
        full rationale (bounding-box-first, then per-component nearest-aspect lookup summed
        and Stefan-Boltzmann-scaled by that component's current temperature).
        """
        return self._cpp.ir_detect(t, sensor_position, sensor_orientation,
                                    target._cpp, target_position, target_orientation)

    def perceive(self, signature, fields, sensor_strength=1.0, falloff_exponent=2.0):
        """Turns a raw signature (radar_detect's or ir_detect's return value, which
        includes "range_km") into what this aircraft's own sensor actually picks up:
        range dropoff combined with this sensor's own strength/sensitivity multiplier,
        via the C++ backend. falloff_exponent=4 for radar (round-trip radar equation),
        =2 (the default) for IR (one-way, inverse-square).
        """
        range_km = signature["range_km"]
        return {
            field: CppAircraft.perceive(signature[field], range_km, sensor_strength, falloff_exponent)
            for field in fields
        }


def _try_float(value):
    try:
        return float(value)
    except ValueError:
        return value
