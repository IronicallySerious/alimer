#version 450

#extension GL_GOOGLE_include_directive : enable

#include "alimer.glsl"

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main()
{
	gl_Position = camera.projectionMatrix * camera.viewMatrix * vec4(inPosition.xyz, 1.0);
    //gl_Position = vec4(inPosition.xyz, 1.0);
	outColor = inColor;
}
