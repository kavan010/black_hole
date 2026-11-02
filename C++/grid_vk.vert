#version 450
layout(location = 0) in vec3 aPos;
layout(push_constant) uniform PC { mat4 viewProj; } pc;
void main() {
    gl_Position = pc.viewProj * vec4(aPos, 1.0);
}
