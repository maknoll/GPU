

varying vec4 diffuse, ambient;
varying vec3 normal, lightDir;

attribute vec3 in_Position;
attribute vec3 in_Normal;
attribute vec2 in_TexCoord;

void main()
{
	vec4 vrcPos = gl_ModelViewMatrix * vec4(in_Position,1);
	vrcPos.xyz /= vrcPos.w;

	normal = gl_NormalMatrix * in_Normal;	

	lightDir = gl_LightSource[0].position.xyz - vrcPos.xyz;

	diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
	ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;

	gl_Position = gl_ProjectionMatrix * vrcPos;
}
