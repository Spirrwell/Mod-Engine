#version 460

layout ( binding = 0 ) uniform UniformBufferObject
{
	mat4 mvp;
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
layout ( location = 2 ) in vec2 inTexCoord;

layout ( location = 0 ) out vec2 fragTexCoord;
layout ( location = 1 ) out vec4 fragColor;

void main()
{
	gl_Position = ubo_mvp.mvp * vec4( inPosition, 1.0 );
	
	fragColor = inColor;
	fragTexCoord = inTexCoord;
}
