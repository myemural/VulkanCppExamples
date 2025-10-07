struct VSInput
{
    [[vk::location(0)]] float3 pos : POSITION;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
};

struct UBO
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

[[vk::binding(1, 0)]] ConstantBuffer<UBO> ubo;

struct VSOutput
{
    float4 Position : SV_POSITION;
    [[vk::location(0)]] float2 Uv : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.Position = mul(ubo.projection, mul(ubo.view, mul(ubo.model, float4(input.pos, 1.0))));
    output.Uv = input.uv;
    return output;
}