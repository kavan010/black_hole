#version 330 core
out vec4 FragColor;

void main()
{
    // A semi-transparent white is a good default for a grid
    FragColor = vec4(1.0, 1.0, 1.0, 0.5);
}
