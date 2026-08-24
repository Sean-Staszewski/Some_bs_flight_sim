from aircraft_cpp import CppAircraft
from Aircraft.Missile.Missile import Missile


class Aim120(Missile):
    def __init__(self):
        super().__init__("AIM_120", "Raytheon")

    def _create_cpp_backend(self):
        return CppAircraft.aim120()
