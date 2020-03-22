#version 460

layout ( binding = 0 ) uniform UniformBufferObject
{
	mat4 mvp;
} ubo_mvp;

layout ( location = 0 ) in vec3 inPosition;
layout ( location = 1 ) in vec2 inTexCoord;
layout ( location = 2 ) in vec3 inColor;

layout ( location = 0 ) out vec2 fragTexCoord;
layout ( location = 1 ) out vec3 fragColor;

void main()
{
	gl_Position = ubo_mvp.mvp * vec4( inPosition, 1.0 );
	
	fragColor = inColor;
	fragTexCoord = inTexCoord;
}
