// Unit tests for Sensor.cpp's sense() -- the radar detection function that decides
// which aircraft in a list fall inside a sensor's range + FOV cone (or full sphere).
// No CSV signature fixtures are needed: detection (whether an aircraft shows up as a
// key in the result) doesn't depend on getSignature() finding a real signature file --
// a missing/unreadable CSV just yields an empty per-aircraft value map, which these
// tests don't inspect.
#define CATCH_CONFIG_MAIN
#include "../third_party/catch2/catch.hpp"
#include "../Aircraft/Aircraft.h"

namespace {

Aircraft makeAircraftAt(glm::vec3 position) {
    Aircraft aircraft;
    aircraft.setPosition(position);
    aircraft.path = "Test_Suite/fixtures/no_such_aircraft"; // deliberately missing signature data
    return aircraft;
}

const glm::vec3 kSensorPos(0.0f, 0.0f, 0.0f);
const glm::mat4 kSensorOrientation(1.0f); // identity: forward=+X, up=+Y, right=+Z

} // namespace

TEST_CASE("target directly ahead, in range and FOV, is detected", "[radar]") {
    vector<Aircraft> targets = { makeAircraftAt(glm::vec3(10.0f, 0.0f, 0.0f)) };
    auto result = sense(kSensorPos, kSensorOrientation, 60.0f, 60.0f, 50.0f, "radar", targets);
    REQUIRE(result.count(&targets[0]) == 1);
}

TEST_CASE("target behind the sensor is not detected", "[radar]") {
    vector<Aircraft> targets = { makeAircraftAt(glm::vec3(-10.0f, 0.0f, 0.0f)) };
    auto result = sense(kSensorPos, kSensorOrientation, 60.0f, 60.0f, 50.0f, "radar", targets);
    REQUIRE(result.empty());
}

TEST_CASE("target beyond max range is not detected", "[radar]") {
    vector<Aircraft> targets = { makeAircraftAt(glm::vec3(100.0f, 0.0f, 0.0f)) };
    auto result = sense(kSensorPos, kSensorOrientation, 60.0f, 60.0f, 50.0f, "radar", targets);
    REQUIRE(result.empty());
}

TEST_CASE("target outside the FOV cone is not detected", "[radar]") {
    vector<Aircraft> targets = { makeAircraftAt(glm::vec3(10.0f, 0.0f, 20.0f)) };
    auto result = sense(kSensorPos, kSensorOrientation, 20.0f, 20.0f, 50.0f, "radar", targets);
    REQUIRE(result.empty());
}

TEST_CASE("an aircraft behind the sensor does not block detection of others", "[radar]") {
    vector<Aircraft> targets = {
        makeAircraftAt(glm::vec3(-10.0f, 0.0f, 0.0f)), // behind
        makeAircraftAt(glm::vec3(10.0f, 0.0f, 0.0f)),  // ahead
    };
    auto result = sense(kSensorPos, kSensorOrientation, 60.0f, 60.0f, 50.0f, "radar", targets);
    CHECK(result.count(&targets[0]) == 0);
    CHECK(result.count(&targets[1]) == 1);
}

TEST_CASE("full-sphere sensor detects a target behind it", "[radar]") {
    vector<Aircraft> targets = { makeAircraftAt(glm::vec3(-10.0f, 0.0f, 0.0f)) };
    auto result = sense(kSensorPos, kSensorOrientation, 360.0f, 360.0f, 50.0f, "radar", targets);
    REQUIRE(result.count(&targets[0]) == 1);
}

TEST_CASE("non-positive range yields no detections", "[radar]") {
    vector<Aircraft> targets = { makeAircraftAt(glm::vec3(1.0f, 0.0f, 0.0f)) };
    auto result = sense(kSensorPos, kSensorOrientation, 60.0f, 60.0f, 0.0f, "radar", targets);
    REQUIRE(result.empty());
}
