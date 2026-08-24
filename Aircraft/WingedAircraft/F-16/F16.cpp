#include "F16.h"

#include <cmath>

F16::F16() : WingedAircraft("F-16", "General Dynamics")
{
}

std::vector<SectorVolume> F16::radarCoverage(float t) const
{
    float fovDeg = radarConfig.azimuthDeg;
    float phase = std::fmod(t, radarSweepPeriodS) / radarSweepPeriodS;
    float sweep = std::abs(2.0f * phase - 1.0f); // 0 -> 1 -> 0 across the period
    float beamCenterDeg = -fovDeg / 2.0f + sweep * fovDeg;

    SectorVolume v;
    v.name = "primary";
    v.centerAzimuthDeg = beamCenterDeg;
    v.azimuthWidthDeg = radarBeamWidthDeg;
    v.centerElevationDeg = 0.0f;
    v.elevationWidthDeg = radarConfig.elevationDeg;
    v.rangeKm = radarConfig.maxRangeKm;
    return {v};
}
