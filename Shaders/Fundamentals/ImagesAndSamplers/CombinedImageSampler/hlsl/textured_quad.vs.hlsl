struct VSInput
{
    [[vk::location(0)]] float2 pos : POSITION;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    [[vk::location(0)]] float2 Uv : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.Position = float4(input.pos, 0.0, 1.0);
    output.Uv = input.uv;
    return output;
}