import json
from pathlib import Path

from type_loader import load_subclass

DATA_ROOT = Path(__file__).resolve().parent


class Armament:
    def __init__(self, name):
        self.name = name
        self.armament_path = DATA_ROOT / name
        self.armament_data_path = self.armament_path / "data"
        self.specs = self._load_armament_json("specs.json")

    def _load_armament_json(self, filename):
        with open(self.armament_data_path / filename) as f:
            return json.load(f)


def load_armament_type(name):
    """The class file lives in the armament's own Armaments/<name>/ dir, unless it's also
    an Aircraft (e.g. a Missile), in which case it lives in a <name>/ dir somewhere under
    the Aircraft/ tree instead (at whatever depth its intermediary class lives at)."""
    from Aircraft.Aircraft import DATA_ROOT as AIRCRAFT_DATA_ROOT

    for aircraft_style_dir in AIRCRAFT_DATA_ROOT.rglob(name):
        if aircraft_style_dir.is_dir() and list(aircraft_style_dir.glob("*.py")):
            return load_subclass(aircraft_style_dir, Armament)
    return load_subclass(DATA_ROOT / name, Armament)
