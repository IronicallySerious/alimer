#version 450

layout (location = 0) in mediump vec4 vColor;
layout (location = 1) in highp vec2 vTex;

layout (location = 0) out mediump vec4 outFragColor;

void main() 
{
	mediump vec4 color = vColor;
	outFragColor = color;
}
