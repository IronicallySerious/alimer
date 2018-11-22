#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec4 inColor;

layout (binding = 0) uniform Projection
{
    mat4 projection;
};

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 outUV;

out gl_PerVertex 
{
    vec4 gl_Position;
};

vec3 SrgbToLinear(vec3 srgb)
{
    return srgb * (srgb * (srgb * 0.305306011 + 0.682171111) + 0.012522878);
}

void main() 
{
    gl_Position = projection * vec4(inPosition, 0, 1);
    outColor = vec4(SrgbToLinear(inColor.rgb), inColor.a);
    outUV = inUV;
}
