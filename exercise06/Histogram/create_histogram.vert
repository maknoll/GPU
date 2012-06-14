uniform sampler2D imageTexture;

const float SCREEN_WIDTH = 512.0;

void main()
{	
	// TODO: Farbe auslesen
	vec2 TexCoord = gl_Vertex.xy / SCREEN_WIDTH;
	vec3 color = texture2D(imageTexture, TexCoord.st).rgb;
 	
	// TODO: Grauwert berechnen
	float greyscale = 0.299 * color.r + 0.587 * color.g + 0.114 * color.b;

	// TODO: x-Position berechnen. Das Zielpixel ist zwischen (0,0) und (255,0)
	float Xposition = greyscale * 255.0;
	
	// TODO: Die Position in [0,1] auf das Intervall [-1,1] abbilden.
	vec2 Vertex = vec2(0);
	Vertex.x = (Xposition - SCREEN_WIDTH / 2.0) / (SCREEN_WIDTH / 2.0);

	gl_Position = vec4(Vertex.x, -1.0, 0.0, 1.0);
}
