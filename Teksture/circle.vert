#version 330 core

layout(location = 0) in vec3 aPos;

out vec2 fragPos; // Prosleđujemo poziciju u fragment sejder

void main()
{
    gl_Position = vec4(aPos, 1.0); // Pozicija
    fragPos = aPos.xy;             // cuvamo poziciju za fragment sejder
}
