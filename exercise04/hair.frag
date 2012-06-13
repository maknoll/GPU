// simple fragment shader that outputs transparent white (as hair color)

#version 120

// out vec4 fragColor;

void main()
{		
	gl_FragColor = vec4(0.75, 0.375, 0.075, 1);
}
