#version 460

layout( location = 0 ) out vec4 outColor;

layout( location = 0 ) in vec3 fragColor; // Vertex color

void main()
{
	outColor = vec4( fragColor, 1.0 );
}