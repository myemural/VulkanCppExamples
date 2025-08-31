#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragUV;
layout(location = 1) flat in int samplerIdx;

layout(set = 0, binding = 0) uniform texture2D uTex;
layout(set = 0, binding = 1) uniform sampler uSamplers[4];

void main()
{
    outColor = texture(sampler2D(uTex, uSamplers[samplerIdx]), fragUV);
}