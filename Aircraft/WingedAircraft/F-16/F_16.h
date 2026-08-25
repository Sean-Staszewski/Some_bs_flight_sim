#include "../WingedAircraft.h"

class F_16 : public WingedAircraft {
public:
    F_16(glm::vec3 initialPosition, glm::mat4 initialOrientation);

    ~F_16();

    F_16(const F_16& source);

    F_16& operator=(const F_16& source);
};