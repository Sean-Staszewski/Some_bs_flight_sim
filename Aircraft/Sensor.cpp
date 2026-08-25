#include "Sensor.h"
#include "Aircraft.h"
#include <unordered_map>
#include <fstream>
#include <sstream>

using namespace std;

void Sensor::sweep(float dt) {
    if (sweepLeft) {
        sweepingAngle -= sweepSpeed * dt;
        if (sweepingAngle < -maxSweepingAngle) {
            float t_to_max = (-maxSweepingAngle + sweepingAngle) / sweepSpeed;
            sweepingAngle = -maxSweepingAngle + sweepSpeed * (dt - t_to_max); 
            sweepLeft = false; // Change direction
        }
    } else {
        sweepingAngle += sweepSpeed * dt;
        if (sweepingAngle > maxSweepingAngle) {
            float t_to_max = (maxSweepingAngle - sweepingAngle) / sweepSpeed;
            sweepingAngle = maxSweepingAngle - sweepSpeed * (dt - t_to_max); 
            sweepLeft = true; // Change direction
        }
    }
}

// Loads a signature CSV shaped like "x,y,z,<value columns...>" and returns the row
// whose (x,y,z) direction is closest (max dot product) to `direction`. Every row in
// these files is already a unit vector, so no normalization is needed on that side.
unordered_map<string, float> nearestSignatureRow(const std::string& csvPath, const glm::vec3& direction) {
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        return {};
    }

    std::string headerLine;
    std::getline(file, headerLine);
    std::vector<std::string> columns;
    std::stringstream headerStream(headerLine);
    std::string columnName;
    while (std::getline(headerStream, columnName, ',')) {
        columns.push_back(columnName);
    }

    glm::vec3 target = glm::normalize(direction);

    std::unordered_map<std::string, float> bestValues;
    float bestDot = -2.0f; // dot products are at most 1.0, so anything found beats this

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream lineStream(line);
        std::string cell;
        std::unordered_map<std::string, float> row;
        for (size_t i = 0; i < columns.size() && std::getline(lineStream, cell, ','); ++i) {
            row[columns[i]] = std::stof(cell);
        }
        if (row.count("x") == 0 || row.count("y") == 0 || row.count("z") == 0) {
            continue;
        }

        glm::vec3 rowDirection(row.at("x"), row.at("y"), row.at("z"));
        float dot = glm::dot(rowDirection, target);
        if (dot > bestDot) {
            bestDot = dot;
            row.erase("x");
            row.erase("y");
            row.erase("z");
            bestValues = std::move(row);
        }
    }

    return bestValues;
}

// Returns true if pos2 is within this sensor's coverage volume as seen from pos1
// (with the sensor pointed per `orientation`), false otherwise. A full 360x360 FOV is
// a sphere; anything narrower is a cone whose cross-section at any range is an oval
// sized by horizontalFOV/verticalFOV -- not a box formed by independently checking
// azimuth and elevation bands (that's what SectorVolume::contains in Sensing.h does).
unordered_map<const Aircraft*, unordered_map<string, float>> sense(const glm::vec3& pos1, glm::mat4 orientation, float horizontalFOV,
            float verticalFOV, float range, string type, const vector<Aircraft>& aircrafts)
{
    unordered_map<const Aircraft*, unordered_map<string, float>> sensed;

    if (range <= 0.0f) {
        return {}; // Invalid range
    }

    if (horizontalFOV <= 0.0f || verticalFOV <= 0.0f) {
        return {}; // Invalid FOV
    }

    for (const Aircraft& aircraft : aircrafts) {

        glm::vec3 pos2 = aircraft.getPosition();

        glm::vec3 delta = pos2 - pos1;
        float distanceSquared = glm::dot(delta, delta); // avoids the sqrt in glm::length

        if (distanceSquared > range * range) {
            continue; // this aircraft is out of range -- others may still be in range
        }

        if (horizontalFOV >= 360.0f && verticalFOV >= 360.0f) {
            sensed[&aircraft] = getSignature(delta, type, aircraft); // full-sphere sensor
            continue;
        }

        // Local axes: +X nose (boresight), +Y up, +Z right -- same convention as the rest
        // of the sensing code. orientation is local -> world, so it maps these directly
        // into world space (no inverse needed here, unlike rotating delta the other way).
        glm::vec3 forwardAxis = glm::vec3(orientation * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
        glm::vec3 upAxis      = glm::vec3(orientation * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
        glm::vec3 rightAxis   = glm::vec3(orientation * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));

        float forward = glm::dot(delta, forwardAxis);
        if (forward <= 0.0f) {
            continue; // behind the sensor -- can't be inside a forward-facing cone, but other aircraft might still be visible
        }

        float horizontalOffset = glm::dot(delta, rightAxis);
        float verticalOffset   = glm::dot(delta, upAxis);

        // At `forward` distance along the boresight, the cone's cross-section is an
        // ellipse with half-axes forward*tan(halfH) and forward*tan(halfV) -- that's the
        // actual base of a cone, unlike checking bearing/elevation angles directly (which
        // only approximates it). pos2 is inside the cone if it falls within that ellipse.
        float halfHorizontalRad = glm::radians(horizontalFOV / 2.0f);
        float halfVerticalRad   = glm::radians(verticalFOV / 2.0f);
        float horizontalExtent = forward * std::tan(halfHorizontalRad);
        float verticalExtent   = forward * std::tan(halfVerticalRad);

        float normalizedHorizontal = horizontalOffset / horizontalExtent;
        float normalizedVertical   = verticalOffset / verticalExtent;
        float ellipseValue = normalizedHorizontal * normalizedHorizontal + normalizedVertical * normalizedVertical;

        if (ellipseValue <= 1.0f) {
            sensed[&aircraft] = getSignature(delta, type, aircraft); // inside the cone
        }
    }
    
    return sensed;
}

unordered_map<string, float> getSignature(glm::vec3 direction, string type, const Aircraft& aircraft) {

    string path;
    if (type == "radar") {
        path = aircraft.path + "/data/radar_signature/sig.csv";
    } else if (type == "infrared") {
        path = aircraft.path + "/data/ir_signature/sig.csv";
    } else {
        // Unknown sensor type
        return {};
    }

    return nearestSignatureRow(path, direction);
}