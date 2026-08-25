CXX      = g++
CXXFLAGS = -std=c++17 -Wall -I../third_party/glm

# These tests link against the Aircraft/Sensor logic directly -- no GL context,
# window, or rendering is needed, only the ObjInstance/Aircraft type declarations
# (via Renderer/scene_types.h, which needs GL/glew.h purely for its declarations).
# Uses the vendored Catch2 v2 single header (../third_party/catch2/catch.hpp);
# CATCH_CONFIG_MAIN in each test's .cpp generates main(), so nothing else to link.

TESTS = radar_detection_test

TEST_3D      = ../Renderer/main.cpp ../Renderer/glutils.cpp ../Renderer/controls.cpp ../Renderer/scene_types.cpp \
           ../Aircraft/WingedAircraft/WingedAircraft.cpp \
           ../Aircraft/WingedAircraft/F-16/F_16.cpp
		   radar_detection_test_3D.cpp


all: $(TESTS)

radar_detection_test: radar_detection_test.cpp ../Aircraft/Sensor.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

radar_detection_test_with_rendering: ../Aircraft/Sensor.cpp $(TEST_3D)
	$(CXX) $(CXXFLAGS) -o $@ $^

run: $(TESTS)
	@status=0; \
	for t in $(TESTS); do \
		echo "== $$t =="; \
		./$$t || status=1; \
		echo; \
	done; \
	exit $$status

clean:
	rm -f $(TESTS)

.PHONY: all run clean
