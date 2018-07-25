#version 450

#if HAVE_VERTEX_COLOR
layout (location = 0) in mediump vec4 vColor;
#endif

layout (location = 1) in highp vec2 vTexCoord;
layout (location = 0) out mediump vec4 outFragColor;

layout(binding = 0) uniform texture2D colorMap;
layout(binding = 1) uniform sampler colorMapSampler;

void main()
{
#if HAVE_VERTEX_COLOR
    mediump vec4 baseColor = texture(sampler2D(colorMap, colorMapSampler), vTexCoord) * vColor;
#else
    mediump vec4 baseColor = texture(sampler2D(colorMap, colorMapSampler), vTexCoord);
#endif

	outFragColor = baseColor;
}
