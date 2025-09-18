struct PSInput
{
    [[vk::location(0)]] float3 color : COLOR;
};

float4 main(PSInput input) : SV_Target
{
    return float4(input.color, 1.0);
}