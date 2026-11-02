#version 450
layout(location = 0) in vec2 TexCoord;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 4) uniform sampler2D screenTexture;
void main() {
    FragColor = texture(screenTexture, TexCoord);
}
