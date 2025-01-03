    #version 330 core
    layout(location = 0) in vec3 aPos;
    uniform float aspectRatio;

    void main() {
        vec3 scaledPos = vec3(aPos.x / aspectRatio, aPos.y, aPos.z);
        gl_Position = vec4(scaledPos, 1.0);
    }