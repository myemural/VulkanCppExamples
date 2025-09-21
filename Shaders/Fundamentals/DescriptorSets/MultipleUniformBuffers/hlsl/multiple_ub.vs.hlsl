struct VSInput
{
    [[vk::location(0)]] float2 pos : POSITION;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.Position = float4(input.pos, 0.0, 1.0);
    return output;
}