from aircraft_cpp import CppAircraft
from Aircraft.WingedAircraft.WingedAircraft import WingedAircraft


class F16(WingedAircraft):
    def __init__(self):
        super().__init__("F-16", "General Dynamics")

    def _create_cpp_backend(self):
        # radar sweep period/beam width default to the same 4.0s/3.0deg this used to
        # hardcode in Python's now-removed radar_coverage override -- see F16.h/F16.cpp.
        return CppAircraft.f16()
