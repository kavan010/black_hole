#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D image;
uniform bool horizontal;  // true for horizontal, false for vertical
uniform float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

/**
 * Separable Gaussian Blur
 * Uses two passes (horizontal + vertical) for efficiency
 */
void main() {
    vec2 tex_offset = 1.0 / textureSize(image, 0);  // Size of single texel
    vec3 result = texture(image, TexCoord).rgb * weight[0];  // Current fragment

    if (horizontal) {
        for (int i = 1; i < 5; ++i) {
            result += texture(image, TexCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, TexCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    } else {
        for (int i = 1; i < 5; ++i) {
            result += texture(image, TexCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(image, TexCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }

    FragColor = vec4(result, 1.0);
}
