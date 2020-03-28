#version 460

layout( location = 0 ) out vec4 outColor;

layout( location = 0 ) in vec4 fragColor; // Vertex color

void main()
{
	outColor = fragColor;
}