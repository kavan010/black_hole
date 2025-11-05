#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D hdrTexture;
uniform float threshold;  // Brightness threshold for bloom

/**
 * Bloom Extraction Pass
 * Extracts bright areas from HDR texture that exceed threshold
 */
void main() {
    vec3 color = texture(hdrTexture, TexCoord).rgb;

    // Calculate brightness (luminance)
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));

    // Only output colors brighter than threshold
    if (brightness > threshold) {
        FragColor = vec4(color, 1.0);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
