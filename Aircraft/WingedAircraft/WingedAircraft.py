from Aircraft.Aircraft import Aircraft


class WingedAircraft(Aircraft):
    def __init__(self, name, manufacturer):
        super().__init__(name, manufacturer)
        self.physics = self._load_json("physics.json")
