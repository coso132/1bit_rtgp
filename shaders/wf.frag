#version 410 core

uniform vec4 uWireColor = vec4(1.0, 1.0, 1.0, 1.0);
out vec4 FragColor;

void main()
{
    FragColor = uWireColor;
}
