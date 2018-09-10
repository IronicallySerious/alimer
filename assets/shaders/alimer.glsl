#ifndef ALIMER_GLSL
#define ALIMER_GLSL

layout (set = 0, binding = 0) uniform AlimerPerCamera
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
} camera;

layout (set = 0, binding = 1) uniform AlimerPerDraw
{
	mat4 worldMatrix;
};

#endif
