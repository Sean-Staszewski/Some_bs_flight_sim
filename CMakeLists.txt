cmake_minimum_required(VERSION 3.15)
project(SomeBsFlightSim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(aircraft_physics
    Aircraft/Aircraft.cpp
    Aircraft/Sensing.cpp
    Aircraft/WingedAircraft/WingedAircraft.cpp
    Aircraft/WingedAircraft/F-16/F16.cpp
    Aircraft/Missile/Missile.cpp
    Aircraft/Missile/AIM_120/Aim120.cpp
)

target_include_directories(aircraft_physics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glm
)

# Plain C ABI + ctypes, not pybind11: this environment's only C++ compiler (MinGW
# win32-threads) can't build pybind11, which needs std::mutex/std::call_once
# unconditionally. See Aircraft/capi.h for details. Aircraft/bindings.cpp (pybind11)
# is kept for whenever a posix-threads/MSVC toolchain is available -- enable it with
# -DBUILD_PYTHON_BINDINGS=ON at that point.
add_library(aircraft_capi SHARED Aircraft/capi.cpp)
target_link_libraries(aircraft_capi PRIVATE aircraft_physics)
# Static-link the MinGW runtime so the DLL has no external dependency to resolve --
# ctypes.CDLL() has no notion of the compiler's bin dir being on PATH.
target_link_options(aircraft_capi PRIVATE -static-libgcc -static-libstdc++)

option(BUILD_PYTHON_BINDINGS "Build the aircraft_cpp pybind11 Python extension module" OFF)
if(BUILD_PYTHON_BINDINGS)
    find_package(Python COMPONENTS Interpreter Development REQUIRED)
    execute_process(
        COMMAND "${Python_EXECUTABLE}" -c "import pybind11; print(pybind11.get_cmake_dir())"
        OUTPUT_VARIABLE pybind11_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    find_package(pybind11 CONFIG REQUIRED)

    pybind11_add_module(aircraft_cpp Aircraft/bindings.cpp)
    target_link_libraries(aircraft_cpp PRIVATE aircraft_physics)
endif()
