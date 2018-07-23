#version 450

#include "alimer.glsl"

layout (location = 0) in highp vec3 inPosition;
layout (location = 1) in mediump vec4 inColor;
layout (location = 2) in highp vec2 inTexCoord;

layout (location = 0) out mediump vec4 vColor;
layout (location = 1) out highp vec2 vTex;

void main()
{
	gl_Position = camera.projectionMatrix * camera.viewMatrix * vec4(inPosition.xyz, 1.0);
    //gl_Position = vec4(inPosition.xyz, 1.0);
	vColor = inColor;
	vTex = inTexCoord;
}
