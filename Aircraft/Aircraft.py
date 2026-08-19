import csv
import json
from dataclasses import dataclass
from pathlib import Path

from Armaments.Armament import Armament, load_armament_type
from Countermeasures.Countermeasure import Countermeasure, load_countermeasure_type

DATA_ROOT = Path(__file__).resolve().parent


@dataclass
class SectorVolume:
    name: str
    center_azimuth_deg: float
    azimuth_width_deg: float
    center_elevation_deg: float
    elevation_width_deg: float
    range_km: float

    def contains(self, bearing_deg, elevation_deg, range_km=None):
        az_ok = _within_sector(bearing_deg - self.center_azimuth_deg, self.azimuth_width_deg)
        el_ok = _within_sector(elevation_deg - self.center_elevation_deg, self.elevation_width_deg)
        range_ok = range_km is None or range_km <= self.range_km
        return az_ok and el_ok and range_ok


class Aircraft:
    def __init__(self, name, manufacturer):

        self.name = name
        self.manufacturer = manufacturer
        self.path = DATA_ROOT / name
        self.data_path = self.path / "data"

        self.radar = self._load_json("radar.json")
        self.ir_sensor = self._load_json("ir_sensor.json")
        self.radio = self._load_json("radio.json")

        self.rcs_signature = self._load_table_csv(self.data_path / "rcs_signature.csv")
        self.ir_signature = self._load_table_csv(self.data_path / "ir_signature.csv")

        self.fuel_tables = self._load_table_dir("fuel")
        self.temperature_tables = self._load_table_dir("temperature")

        self.armaments: dict[Armament, int] = self._load_loadout("armaments", load_armament_type)
        self.countermeasures: dict[Countermeasure, int] = self._load_loadout("countermeasures", load_countermeasure_type)

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

    def radar_coverage(self, t):
        """Default: static sector centered on the nose. Override per type for sweeping/scanning radars."""
        fov = self.radar["field_of_regard_deg"]
        return [self._static_sector("primary", self.radar["max_range_km"], fov)]

    def ir_coverage(self, t):
        """Default: one static sector per named sub-sensor in ir_sensor.json (e.g. primary, early_warning)."""
        return [
            self._static_sector(name, cfg["max_range_km"], cfg["field_of_view_deg"])
            for name, cfg in self.ir_sensor.items()
        ]

    def _static_sector(self, name, range_km, fov):
        return SectorVolume(
            name=name,
            center_azimuth_deg=0.0,
            azimuth_width_deg=fov["azimuth"],
            center_elevation_deg=0.0,
            elevation_width_deg=fov["elevation"],
            range_km=range_km,
        )


def _within_sector(bearing_deg, width_deg):
    bearing_deg = (bearing_deg + 180) % 360 - 180
    return abs(bearing_deg) <= width_deg / 2


def _try_float(value):
    try:
        return float(value)
    except ValueError:
        return value
