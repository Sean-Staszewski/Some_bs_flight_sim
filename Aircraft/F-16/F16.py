from Aircraft.Aircraft import Aircraft, SectorVolume


class F16(Aircraft):
    def __init__(self):
        super().__init__("F-16", "General Dynamics")
        self._radar_sweep_period_s = 4.0
        self._radar_beam_width_deg = 3.0

    def radar_coverage(self, t):
        fov = self.radar["field_of_regard_deg"]
        phase = (t % self._radar_sweep_period_s) / self._radar_sweep_period_s
        sweep = abs(2 * phase - 1)  # 0 -> 1 -> 0 across the period
        beam_center_deg = -fov["azimuth"] / 2 + sweep * fov["azimuth"]

        return [SectorVolume(
            name="primary",
            center_azimuth_deg=beam_center_deg,
            azimuth_width_deg=self._radar_beam_width_deg,
            center_elevation_deg=0.0,
            elevation_width_deg=fov["elevation"],
            range_km=self.radar["max_range_km"],
        )]
