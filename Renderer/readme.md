Dependencies:
GLEW, GLFW, GLM, and OpenGL/X11 for linux

Run the command below if dependencies are not satisfied:
sudo apt update
sudo apt install -y \
    build-essential \
    libglew-dev \
    libglfw3-dev \
    libglm-dev \
    libgl1-mesa-dev \
    libx11-dev \
    libxi-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev




To run:
make
./demo


Controls:

Camera:
W: Forwards
A: left
S: Back
D: Right
Ctrl: Down
Space: Up
Mouse: Click and drag to change view direction, Scroll wheel to change Camera depth

Objects:
Toggle select object 1 or 2 by pressing the 1 or 2 key.
W: Forwards
A: left
S: Back
D: Right
Ctrl: Down
Space: Up
Mouse: Click and drag to change object orientation, Scroll wheel to change object scaling

Miscellanious:
Z: Z-Buffer toggle

