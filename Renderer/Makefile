CXX      = g++
CXXFLAGS = -std=c++17 -Wall -fpermissive

# Libraries
LIBS     = -lGL -lGLEW -lglfw

# GLM is header-only; just needs to be on the include path.
# If installed via package manager (e.g. libglm-dev) this is automatic.
# Otherwise: CXXFLAGS += -I/path/to/glm

TARGET   = demo
SRC      = main.cpp glutils.cpp controls.cpp scene_types.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET)