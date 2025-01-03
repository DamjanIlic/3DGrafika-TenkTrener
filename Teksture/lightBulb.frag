#version 330 core
    out vec4 FragColor;
    uniform float colorSwitch;

    void main() {
                                                                                                  //SIJALICA
        if (colorSwitch == 0.0) {
            FragColor = vec4(0.5, 0.5, 0.5, 1.0); // Grlo sijalice // SIVA
        } else if (colorSwitch == 1.0) {
            FragColor = vec4(1.0, 1.0, 0.0, 1.0); // Glava sijalice // ZUTA
        } else if (colorSwitch == 2.0) {
            FragColor = vec4(1.0, 1.0, 0.5, 0.5); // Svetlo zuta (aurora) // BLAGO ZUTA                
        } else if (colorSwitch == 3.0){
            FragColor = vec4(1.0, 1.0, 0.5, 0.5); //
        } else if (colorSwitch == 4.0){
            FragColor = vec4(0.2f, 0.2f, 0.2f, 1.0f);
        }
        

                                                                                                    //IGLA VOLTMETRA NEKA JE OVDE 
        else if(colorSwitch == 5.0){
            FragColor = vec4(0.1, 0.1, 0.1,1.0); //CRNA 
        }
    }