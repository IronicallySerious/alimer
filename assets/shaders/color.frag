#version 450

layout (location = COLOR0) in vec4 inColor;
layout (location = SV_Target0) out vec4 outFragColor;

void main() 
{
	outFragColor = inColor;
}
