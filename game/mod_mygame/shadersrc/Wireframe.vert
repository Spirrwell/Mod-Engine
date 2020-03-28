#version 460

layout( binding = 0 ) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo_mvp;

/* Vertex Layout Order:
	0: Position
	1: Normal
	2: Color,
	3: UV,
	4: Tangent,
	5: BiTangent,
*/

layout ( location = 0 ) in vec3 inPosition;
layout ( location = 1 ) in vec4 inColor;

layout ( location = 0 ) out vec4 fragColor;

void main()
{
	mat4 mvp = ubo_mvp.proj * ubo_mvp.view * ubo_mvp.model; // Should calculate MVP matrix outside of shader, but meh
	gl_Position = mvp * vec4( inPosition, 1.0 );
	
	fragColor = inColor;
}
