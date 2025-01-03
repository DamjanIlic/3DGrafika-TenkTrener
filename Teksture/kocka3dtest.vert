#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNorm;

out vec2 chTex;
out vec3 vs_normal;
out vec3 vs_position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {

    chTex = aTex;
    vs_normal = mat3(model) * aNorm;
    vs_position = vec4(model * vec4(aPos, 1.f)).xyz;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
