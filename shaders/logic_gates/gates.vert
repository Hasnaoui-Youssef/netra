#version 430 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform vec2 u_position;
uniform vec2 u_size;

out vec2 uv;

void main() {
    vec2 scaled = aPos * u_size * 0.5;
    vec2 positioned = scaled + u_position + vec2(u_size.x * 0.5, -u_size.y * 0.5);
    gl_Position = vec4(positioned, 0.0, 1.0);
    uv = aTexCoord * 2.0 - 1.0;
}
