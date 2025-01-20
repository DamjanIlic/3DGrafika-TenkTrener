#version 330 core

in vec2 chTex; //koordinate teksture
out vec4 outCol;

uniform sampler2D uTex; //teksturna jedinica

void main()
{

	vec4 texColor = texture(uTex, chTex);
	if (texColor.a < 0.1) {
     //   discard; 
    }
	outCol = texColor;
}