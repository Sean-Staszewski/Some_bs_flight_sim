#version 330 core
out vec4 FragColor;

uniform vec3 uWireColor;

void main()
{
    FragColor = vec4(uWireColor, 1.0);
}
