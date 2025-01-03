#version 330 core
in vec2 chTex;
in vec3 vs_normal;
in vec3 vs_position;

out vec4 FragColor;

uniform vec3 lightPos0;
uniform sampler2D uTex;


void main() {

    //AmbientLight
    vec3 ambientLight = vec3(0.025f, 0.025f, 0.025f);
    
    //vs_normal = normalize(vs_normal);
    //Diffuse 
    vec3 posToLightDirVec = normalize(lightPos0  -vs_position );
    vec3 diffuseColor = vec3(.25f, .25f, .25f);
    float diffuse = clamp(dot(posToLightDirVec, vs_normal), 0.0f, 1.0f);
    vec3 diffuseFinal = diffuseColor * diffuse;

    FragColor = vec4(0.9, 0.9, 0.9f, 1.0f)* (vec4(ambientLight, 1.0f) + vec4(diffuseFinal, 1.f));
    //FragColor = texture(uTex, chTex) * (vec4(ambientLight, 1.0f) + vec4(diffuseFinal, 1.f));
    if (FragColor == vec4(0.0, 0.0, 0.0, 1.0)) {
    // Tekstura nije postavljena (prazna)
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);  // Na primer, ispisivanje crvene boje za grešku
    } else {
        FragColor = FragColor;  // Ako je tekstura validna, samo je koristimo
    }
    //FragColor = vec4(chTex.y, chTex.x, chTex.y, 1.0f);
    //FragColor = (texture(uTex,chTex))* vec4(ambientLight, 1.0f) + vec4(diffuseFinal, 1.f));
    //FragColor = vec4(vs_normal, 1.0); // Display diffuse color result

}
