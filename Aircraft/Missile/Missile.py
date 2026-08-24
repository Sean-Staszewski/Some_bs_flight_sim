from Aircraft.Aircraft import Aircraft
from Armaments.Armament import Armament


class Missile(Armament, Aircraft):
    def __init__(self, name, manufacturer):
        Aircraft.__init__(self, name, manufacturer)
        self.specs = self._load_json("specs.json")
