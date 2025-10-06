struct PSInput
{
    [[vk::location(0)]] float2 uv : TEXCOORD0;
};

[[vk::binding(0, 0)]] SamplerState uSampler;
[[vk::binding(0, 0)]] Texture2D uImage;

float4 main(PSInput input) : SV_Target
{
    return uImage.Sample(uSampler, input.uv * 3);
}