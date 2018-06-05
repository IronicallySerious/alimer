#ifndef ALIMER_GLSL
#define ALIMER_GLSL

#define DESCRIPTOR_SET_INDEX_COMMON	0

layout (set = DESCRIPTOR_SET_INDEX_COMMON, binding = 0) uniform AlimerPerCamera
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
} camera;

#endif
