import json
from pathlib import Path

from type_loader import load_subclass

DATA_ROOT = Path(__file__).resolve().parent


class Countermeasure:
    def __init__(self, name):
        self.name = name
        self.path = DATA_ROOT / name
        self.data_path = self.path / "data"
        self.specs = self._load_json("specs.json")

    def _load_json(self, filename):
        with open(self.data_path / filename) as f:
            return json.load(f)


def load_countermeasure_type(name):
    return load_subclass(DATA_ROOT / name, Countermeasure)
