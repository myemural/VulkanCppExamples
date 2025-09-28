#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform sampler2D uCombinedSampler;
layout(set = 0, binding = 1) uniform UBO {
    float time;
    float speed;
} ubo;

void main()
{
    vec2 uv = fragUV;
    uv.x += ubo.time * ubo.speed;

    uv.x = fract(uv.x);

    outColor = texture(uCombinedSampler, uv * 4);
}