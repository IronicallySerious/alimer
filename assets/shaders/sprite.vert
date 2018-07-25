#version 450

#include "alimer.glsl"

layout (location = 0) in highp vec3 inPosition;

layout (location = 2) in highp vec2 inTexCoord;
layout (location = 1) out highp vec2 vTexCoord;

#if HAVE_VERTEX_COLOR
layout (location = 1) in mediump vec4 inColor;
layout (location = 0) out mediump vec4 vColor;
#endif

void main()
{
	gl_Position = camera.projectionMatrix * camera.viewMatrix * vec4(inPosition.xyz, 1.0);

#if HAVE_VERTEX_COLOR
    vColor = inColor;
#endif

    vTexCoord = inTexCoord;
}
