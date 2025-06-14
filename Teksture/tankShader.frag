#version 330 core
in vec2 chTex;
in vec3 vs_normal;
in vec3 vs_position;

out vec4 FragColor;

uniform vec3 moonLightPos;
uniform vec3 lightBulbLightPos;
uniform vec3 canShootLightPos;

uniform int isLightOn;
uniform int isDrawingWindow;
uniform int isDrawingWall;
uniform int canShoot;

uniform sampler2D uTex;


void main() {

    //AmbientLight
    vec3 ambientLight = vec3(0.025f, 0.025f, 0.025f);
    
    //vs_normal = normalize(vs_normal);
    //Diffuse moonLight

    vec3 posToLightDirVec = normalize(moonLightPos  - vs_position );
    
    if(isDrawingWindow == 1){
        posToLightDirVec = normalize(vec3(0.0f, 25.0f, 0.0f - vs_position));
    }
    if(isDrawingWall == 1){
            posToLightDirVec = normalize(vec3(0.0f, 25.0f, 0.0f - vs_position));
    }
    vec3 diffuseColor = vec3(.25f, .25f, .25f);
    float diffuse = clamp(dot(posToLightDirVec, vs_normal), 0.0f, 1.0f);
    vec3 diffuseFinal = diffuseColor * diffuse;

    //Diffuse sijalica u tenku
    posToLightDirVec = normalize(lightBulbLightPos - vs_position);
    diffuseColor = vec3(1.f, 1.f, .6f);
    //diffuseColor = vec3(1f, 1f, 1f);
    diffuse = clamp(dot(posToLightDirVec, vs_normal), 0.0f, 1.0f);

    //diffuseFinal = diffuseFinal1 + diffuseFinal2
    if(isLightOn ==1){ //moonDiffuse + lightBulbDiffuse
        diffuseFinal = diffuseFinal + diffuseColor*diffuse;
    }


    //Can shoot led lampica
    float canShootLightMaxRange = 2.f;
    if(canShoot == 1){
        vec3 posToCanShootLight = canShootLightPos - vs_position;
        float distance = length(posToCanShootLight);
        float rangeIntensity = smoothstep(canShootLightMaxRange, canShootLightMaxRange * 0.5, distance);

        posToLightDirVec = normalize(posToCanShootLight);
        diffuseColor = vec3(1.0f, 0.0f, 0.0f); // Crveno svetlo
        diffuse = clamp(dot(posToLightDirVec, vs_normal), 0.0f, 1.0f);

        diffuseFinal += diffuseColor * diffuse * rangeIntensity; // Intenzitet opada 
    }
    //FragColor = vec4(0.9, 0.9, 0.9f, 0.2f)* (vec4(ambientLight, 0.2f) + vec4(diffuseFinal, 0.2));
    FragColor = texture(uTex, chTex) * (vec4(ambientLight, 1.0f) + vec4(diffuseFinal, 1.f));
    if(isDrawingWindow == 1){
        FragColor = vec4(0.0, 0.3f, .8f, 0.5f)* (vec4(ambientLight, .4) + vec4(diffuseFinal, .4));
        //FragColor == vec4(1.0, 0.0, 0.0, 1.0);
    }
    
    if (FragColor == vec4(0.0, 0.0, 0.0, 1.0)) {
    // Tekstura nije postavljena (prazna)
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);  // Na primer, ispisivanje crvene boje za gresku
    } else {
        FragColor = FragColor;  // Ako je tekstura validna, samo je koristimo
    }

    //FragColor = vec4(chTex.y, chTex.x, chTex.y, 1.0f);
    //FragColor = (texture(uTex,chTex))* vec4(ambientLight, 1.0f) + vec4(diffuseFinal, 1.f));
    //FragColor = vec4(vs_normal, 1.0); // Display diffuse color result

}
