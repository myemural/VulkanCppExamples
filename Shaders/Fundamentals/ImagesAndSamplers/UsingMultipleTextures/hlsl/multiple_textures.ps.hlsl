struct PSInput
{
    [[vk::location(0)]] float2 uv : TEXCOORD0;
    [[vk::location(1)]] nointerpolation int samplerIndex : TEXCOORD1;
};

[[vk::binding(0, 0)]] Texture2D uImages[2];
[[vk::binding(0, 0)]] SamplerState uSamplers[2];

float4 main(PSInput input) : SV_Target
{
    float4 color1 = uImages[0].Sample(uSamplers[0], input.uv); // First texture
    float4 color2 = uImages[1].Sample(uSamplers[1], input.uv); // Second texture

    if (input.samplerIndex == 0) {
        return color1;
    } else if  (input.samplerIndex == 1) {
        return color2;
    }

    return lerp(color1, color2, 0.4); // %60 first, %40 second
}