#version 330 core
in vec2 chTex;
in vec3 vs_normal;
in vec3 vs_position;

out vec4 FragColor;

uniform vec3 lightPos0;
uniform sampler2D uTex;

uniform int isNightVisionOn;
uniform float time;


void main() {

    //AmbientLight
    vec3 ambientLight = vec3(0.01f, 0.01f, 0.01f); //noc
    //vec3 ambientLight = vec3(0.225f, 0.225f, 0.225f); //vajb
    
    //vs_normal = normalize(vs_normal);
    //Diffuse 
    vec3 posToLightDirVec = normalize(lightPos0  -vs_position );
    vec3 diffuseColor = vec3(.075f, .075f, .075f); //noc 
    //vec3 diffuseColor = vec3(.25f, .25f, .25f); //vajb
    if(isNightVisionOn == 1){
       // diffuseColor *= 2.f;
    }
    float diffuse = clamp(dot(posToLightDirVec, vs_normal), 0.0f, 1.0f);
    vec3 diffuseFinal = diffuseColor * diffuse;

    //FragColor = vec4(0.9, 0.9, 0.9f, 1.0f)* (vec4(ambientLight, 1.0f) + vec4(diffuseFinal, 1.f));
    FragColor = texture(uTex, chTex) * (vec4(ambientLight, 1.0f) + vec4(diffuseFinal, 1.f));
    if (FragColor == vec4(0.0, 0.0, 0.0, 1.0)) {
    // Tekstura nije postavljena (prazna)
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);  // Na primer, ispisivanje crvene boje za grešku
    } else {
        //FragColor.y += .5f;
        FragColor = FragColor;  // Ako je tekstura validna, samo je koristimo
    }

    //vec4 baseColor = FragColor;
   // float brightness = dot(FragColor.rgb, vec3(1.299, 1.587, 1.114)); // Luminance formula
   // FragColor.rgb *= smoothstep(0.6, 1, brightness) * 6.0; // Povecaj tamne delove
    //FragColor = FragColor;
        // Dodaj zelenu nijansu
    if(isNightVisionOn == 1){
       // vec3 nightVisionColor = vec3(3., 10., 3.) * FragColor.rgb ;//- ambientLight - diffuseColor;
       vec3 nightVisionColor = vec3(3., 10., 3.) * FragColor.rgb;
        // Dodaj noise efekat
        float noise = fract(sin(dot(gl_FragCoord.xy ,vec2(12.9898,78.233))) * 43758.5453 + time);
        noise = (noise - 0.5) * 0.1; // Skaliranje noisa

        FragColor.rgb = nightVisionColor + vec3(noise);
    }
    //FragColor = baseColor;
    // Dodaj noise efekat
    //float noise = fract(sin(dot(gl_FragCoord.xy ,vec2(12.9898,78.233))) * 43758.5453 + time);
    //noise = (noise - 0.5) * 0.1; // Skaliranje noisa

    //baseColor.rgb = nightVisionColor + vec3(noise);

    //FragColor = vec4(chTex.y, chTex.x, chTex.y, 1.0f);
    //FragColor = (texture(uTex,chTex))* vec4(ambientLight, 1.0f) + vec4(diffuseFinal, 1.f));
    //FragColor = vec4(vs_normal, 1.0); // Display diffuse color result

}
