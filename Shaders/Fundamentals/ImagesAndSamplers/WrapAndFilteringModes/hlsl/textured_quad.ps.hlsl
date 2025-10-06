struct PSInput
{
    [[vk::location(0)]] float2 uv : TEXCOORD0;
    [[vk::location(1)]] nointerpolation int samplerIndex : TEXCOORD1;
};

[[vk::binding(0, 0)]] Texture2D uImage;
[[vk::binding(1, 0)]] SamplerState uSamplers[4];

float4 main(PSInput input) : SV_Target
{
    return uImage.Sample(uSamplers[input.samplerIndex], input.uv);
}