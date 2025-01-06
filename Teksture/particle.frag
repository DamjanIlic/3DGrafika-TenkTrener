#version 330 core

in vec4 oColor; // Boja iz vertex shadera
out vec4 FragColor;

void main() {
    FragColor = oColor;
}
