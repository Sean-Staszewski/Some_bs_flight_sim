#pragma once

#include "../WingedAircraft.h"

class F16 : public WingedAircraft {
public:
    F16();

    // Mechanically-sweeping radar beam: overrides the base's static sector.
    std::vector<SectorVolume> radarCoverage(float t) const override;

    float radarSweepPeriodS = 4.0f;
    float radarBeamWidthDeg = 3.0f;
};
