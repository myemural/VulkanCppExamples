struct VSInput
{
    [[vk::location(0)]] float2 pos : POSITION;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
};

struct PushConstants {
    float2 offset;
    nointerpolation int samplerIndex;
};
[[vk::push_constant]] PushConstants pc;

struct VSOutput
{
    float4 Position : SV_POSITION;
    [[vk::location(0)]] float2 Uv : TEXCOORD0;
    [[vk::location(1)]] nointerpolation int SamplerIndex : TEXCOORD1;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    float2 pos = input.pos + pc.offset;
    output.Position = float4(pos, 0.0, 1.0);
    output.Uv = input.uv;
    output.SamplerIndex = pc.samplerIndex;
    return output;
}