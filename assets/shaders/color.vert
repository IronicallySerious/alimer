#version 450

#include "alimer.glsl"

layout (location = POSITION) in vec3 inPosition;
layout (location = COLOR0) in vec4 inColor;
layout (location = COLOR0) out vec4 outColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	//gl_Position = camera.projectionMatrix * camera.viewMatrix * worldMatrix * vec4(inPosition.xyz, 1.0);
	gl_Position = camera.projectionMatrix * camera.viewMatrix * vec4(inPosition.xyz, 1.0);
    //gl_Position = vec4(inPosition.xyz, 1.0);
	outColor = inColor;
}
