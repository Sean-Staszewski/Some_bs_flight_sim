"""ctypes wrapper over Aircraft/capi.h -- the C++ port of Aircraft.py's temperature
dynamics and sensing math (interpolation, sector containment, aspect-direction matching,
Stefan-Boltzmann scaling, range falloff). Python still owns loading data/*.csv and
*.json; this module just hands that already-parsed data into the compiled C++ and calls
into it for the hot-path math. See capi.h for why this is ctypes and not pybind11.
"""

import ctypes
from pathlib import Path

_DLL_PATH = Path(__file__).resolve().parent / "build" / "libaircraft_capi.dll"
_lib = ctypes.CDLL(str(_DLL_PATH))

_FLOAT_P = ctypes.POINTER(ctypes.c_float)
_INT_P = ctypes.POINTER(ctypes.c_int)

_lib.aircraft_create_f16.restype = ctypes.c_void_p
_lib.aircraft_create_aim120.restype = ctypes.c_void_p
_lib.aircraft_destroy.argtypes = [ctypes.c_void_p]

_lib.aircraft_set_empty_mass_kg.argtypes = [ctypes.c_void_p, ctypes.c_float]
_lib.aircraft_set_fuel_mass_kg.argtypes = [ctypes.c_void_p, ctypes.c_float]
_lib.aircraft_set_armaments_mass_kg.argtypes = [ctypes.c_void_p, ctypes.c_float]
_lib.aircraft_set_countermeasures_mass_kg.argtypes = [ctypes.c_void_p, ctypes.c_float]
_lib.aircraft_update_mass.argtypes = [ctypes.c_void_p]
_lib.aircraft_get_mass_kg.argtypes = [ctypes.c_void_p]
_lib.aircraft_get_mass_kg.restype = ctypes.c_float

_lib.aircraft_set_radar_config.argtypes = [ctypes.c_void_p, ctypes.c_float, ctypes.c_float, ctypes.c_float]
_lib.aircraft_set_ir_sensor_config.argtypes = [
    ctypes.c_void_p, ctypes.c_char_p, ctypes.c_float, ctypes.c_float, ctypes.c_float,
]

_lib.aircraft_set_temperature_table.argtypes = [
    ctypes.c_void_p, ctypes.c_char_p, _FLOAT_P, _FLOAT_P, ctypes.c_int,
]
_lib.aircraft_update_temperature.argtypes = [
    ctypes.c_void_p, ctypes.c_char_p, ctypes.c_float, ctypes.c_float, ctypes.c_float,
]
_lib.aircraft_update_temperature.restype = ctypes.c_float
_lib.aircraft_get_temperature_c.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
_lib.aircraft_get_temperature_c.restype = ctypes.c_float

_lib.aircraft_set_rcs_signature.argtypes = [
    ctypes.c_void_p, _FLOAT_P, _FLOAT_P, _FLOAT_P, _FLOAT_P, _FLOAT_P, ctypes.c_int,
]
_lib.aircraft_set_ir_signature_component.argtypes = [
    ctypes.c_void_p, ctypes.c_char_p, _FLOAT_P, _FLOAT_P, _FLOAT_P, _FLOAT_P, _FLOAT_P, ctypes.c_int,
]

_lib.aircraft_radar_detect.argtypes = [
    ctypes.c_void_p, ctypes.c_float, _FLOAT_P, _FLOAT_P,
    ctypes.c_void_p, _FLOAT_P, _FLOAT_P,
    _INT_P, _FLOAT_P, _FLOAT_P, _FLOAT_P,
]
_lib.aircraft_ir_detect.argtypes = [
    ctypes.c_void_p, ctypes.c_float, _FLOAT_P, _FLOAT_P,
    ctypes.c_void_p, _FLOAT_P, _FLOAT_P,
    _INT_P, _FLOAT_P, _FLOAT_P, _FLOAT_P,
]

_lib.aircraft_perceive.argtypes = [ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float]
_lib.aircraft_perceive.restype = ctypes.c_float


def _float_array(values):
    return (ctypes.c_float * len(values))(*values)


class CppAircraft:
    def __init__(self, handle):
        self._handle = handle

    @classmethod
    def f16(cls):
        return cls(_lib.aircraft_create_f16())

    @classmethod
    def aim120(cls):
        return cls(_lib.aircraft_create_aim120())

    def __del__(self):
        if getattr(self, "_handle", None):
            _lib.aircraft_destroy(self._handle)

    def set_masses(self, empty_kg=0.0, fuel_kg=0.0, armaments_kg=0.0, countermeasures_kg=0.0):
        _lib.aircraft_set_empty_mass_kg(self._handle, empty_kg)
        _lib.aircraft_set_fuel_mass_kg(self._handle, fuel_kg)
        _lib.aircraft_set_armaments_mass_kg(self._handle, armaments_kg)
        _lib.aircraft_set_countermeasures_mass_kg(self._handle, countermeasures_kg)
        _lib.aircraft_update_mass(self._handle)

    @property
    def mass_kg(self):
        return _lib.aircraft_get_mass_kg(self._handle)

    def set_radar_config(self, max_range_km, azimuth_deg, elevation_deg):
        _lib.aircraft_set_radar_config(self._handle, max_range_km, azimuth_deg, elevation_deg)

    def set_ir_sensor_config(self, name, max_range_km, azimuth_deg, elevation_deg):
        _lib.aircraft_set_ir_sensor_config(self._handle, name.encode(), max_range_km, azimuth_deg, elevation_deg)

    def set_temperature_table(self, name, rows, x_key):
        xs = _float_array([r[x_key] for r in rows])
        temps = _float_array([r["temp_c"] for r in rows])
        _lib.aircraft_set_temperature_table(self._handle, name.encode(), xs, temps, len(rows))

    def update_temperature(self, name, x_value, dt, time_constant_s=10.0):
        return _lib.aircraft_update_temperature(self._handle, name.encode(), x_value, dt, time_constant_s)

    def temperature_c(self, name):
        return _lib.aircraft_get_temperature_c(self._handle, name.encode())

    def set_rcs_signature(self, rows):
        xs = _float_array([r["x"] for r in rows])
        ys = _float_array([r["y"] for r in rows])
        zs = _float_array([r["z"] for r in rows])
        xband = _float_array([r["rcs_xband_m2"] for r in rows])
        sband = _float_array([r["rcs_sband_m2"] for r in rows])
        _lib.aircraft_set_rcs_signature(self._handle, xs, ys, zs, xband, sband, len(rows))

    def set_ir_signature_component(self, name, rows):
        xs = _float_array([r["x"] for r in rows])
        ys = _float_array([r["y"] for r in rows])
        zs = _float_array([r["z"] for r in rows])
        mwir = _float_array([r["ir_mwir_w_per_sr"] for r in rows])
        lwir = _float_array([r["ir_lwir_w_per_sr"] for r in rows])
        _lib.aircraft_set_ir_signature_component(self._handle, name.encode(), xs, ys, zs, mwir, lwir, len(rows))

    def radar_detect(self, t, sensor_position, sensor_orientation, target, target_position, target_orientation):
        out_hit = ctypes.c_int()
        out_xband = ctypes.c_float()
        out_sband = ctypes.c_float()
        out_range = ctypes.c_float()
        _lib.aircraft_radar_detect(
            self._handle, t, _float_array(sensor_position), _float_array(sensor_orientation),
            target._handle, _float_array(target_position), _float_array(target_orientation),
            ctypes.byref(out_hit), ctypes.byref(out_xband), ctypes.byref(out_sband), ctypes.byref(out_range),
        )
        if not out_hit.value:
            return None
        return {"rcs_xband_m2": out_xband.value, "rcs_sband_m2": out_sband.value, "range_km": out_range.value}

    def ir_detect(self, t, sensor_position, sensor_orientation, target, target_position, target_orientation):
        out_hit = ctypes.c_int()
        out_mwir = ctypes.c_float()
        out_lwir = ctypes.c_float()
        out_range = ctypes.c_float()
        _lib.aircraft_ir_detect(
            self._handle, t, _float_array(sensor_position), _float_array(sensor_orientation),
            target._handle, _float_array(target_position), _float_array(target_orientation),
            ctypes.byref(out_hit), ctypes.byref(out_mwir), ctypes.byref(out_lwir), ctypes.byref(out_range),
        )
        if not out_hit.value:
            return None
        return {"ir_mwir_w_per_sr": out_mwir.value, "ir_lwir_w_per_sr": out_lwir.value, "range_km": out_range.value}

    @staticmethod
    def perceive(raw_value, range_km, sensor_strength=1.0, falloff_exponent=2.0):
        return _lib.aircraft_perceive(raw_value, range_km, sensor_strength, falloff_exponent)
