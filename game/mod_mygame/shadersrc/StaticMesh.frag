#version 460

layout( location = 0 ) out vec4 outColor;


layout( location = 0 ) in vec2 fragTexCoord;
layout( location = 1 ) in vec4 fragColor; // Vertex color

layout ( binding = 1 ) uniform LightState
{
	vec4 ambientLight;
} lightState;

layout( binding = 2 ) uniform sampler2D diffuseSampler;

void main()
{
	outColor =  texture( diffuseSampler, fragTexCoord ) * fragColor * lightState.ambientLight;
}