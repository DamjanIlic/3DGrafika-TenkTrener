#version 330 core

in vec2 fragPos;   // Uvezeno iz vertex sejdera
out float fragColor; // Izlazni intenzitet (0.0f ili 1.0f)
out vec4 FragColor;
    
// Prima vrednost koja odredjuje koju boju da prikaže
uniform float colorSwitch; 

void main()
{
    float distanceFromCenter = length(fragPos); // Udaljenost od centra (0, 0)

    float radius = 0.5; // Poluprecnik kruga

    // Postavi boju na 1.0 unutar kruga, 0.0 izvan njega
    if (distanceFromCenter <= radius)
        FragColor = vec4(1.0, 1.0, 0.0, 0.2); 
    else
        FragColor =vec4(0.0, 0.0, 1.0, 1.0); 
    
}
