#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 inColor;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec4 fragColor;

layout (binding=0) uniform sampler2D tex;

void main()
{
    fragColor = inColor * texture(tex, inUV);
}
