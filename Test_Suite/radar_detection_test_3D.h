#include "../Renderer/Camera.h"
#include "../Aircraft/Sensor.h"

struct CameraWithRadar : public Camera {

    Sensor radar;

public:
    CameraWithRadar(glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 0.0f));
    
};

void printRadarValues(unordered_map<const Aircraft*, unordered_map<string, float>>& radarResults);