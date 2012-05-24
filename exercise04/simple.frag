
varying vec4 diffuse, ambient;
varying vec3 normal, lightDir;

void main()
{	
	vec3 n = normalize(normal);
	float NdotL = abs(dot(n, normalize(lightDir)));
	gl_FragColor = diffuse * NdotL + ambient;
}
