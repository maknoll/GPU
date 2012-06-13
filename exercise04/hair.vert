#version 120

/*
in vec3 in_Position;
in vec3 in_Normal;

out vec3 normal;

void main()
{	
	normal = in_Normal;
	gl_Position = vec4(in_Position,1);
}
*/

void main() 
{
  gl_Position = ftransform();
  gl_TexCoord[0] = gl_MultiTexCoord0;
}
