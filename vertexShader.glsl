#version 410 core

layout (location = 0) in vec2 vertexPosition;

uniform vec2 screen;
uniform vec3 camPos;

void main() {
    gl_Position = vec4(vertexPosition, 0.0, 1.0);
}
