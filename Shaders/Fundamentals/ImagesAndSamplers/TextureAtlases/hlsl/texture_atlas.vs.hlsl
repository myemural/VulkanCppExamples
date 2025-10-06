struct VSInput
{
    [[vk::location(0)]] float2 pos : POSITION;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
};

struct PushConstants {
    float4 uvRect; // u0,v0,du,dv
    float2 offset;
};
[[vk::push_constant]] PushConstants pc;

struct VSOutput
{
    float4 Position : SV_POSITION;
    [[vk::location(0)]] float2 Uv : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    float2 pos = input.pos + pc.offset;
    output.Position = float4(pos, 0.0, 1.0);
    output.Uv = pc.uvRect.xy + input.uv * pc.uvRect.zw;
    return output;
}