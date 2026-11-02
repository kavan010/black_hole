#version 450
layout(location = 0) out vec2 TexCoord;
void main() {
    // fullscreen quad from gl_VertexIndex (no vertex buffer). v=0 sits at the bottom, like the GL version.
    vec2 p = vec2((gl_VertexIndex == 1 || gl_VertexIndex == 4 || gl_VertexIndex == 5) ? 1.0 : -1.0,
                  (gl_VertexIndex == 2 || gl_VertexIndex == 3 || gl_VertexIndex == 5) ? 1.0 : -1.0);
    gl_Position = vec4(p, 0.0, 1.0);
    TexCoord = vec2(p.x * 0.5 + 0.5, 0.5 - p.y * 0.5);
}
