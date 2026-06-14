// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

struct PSInput
{
    [[vk::location(0)]] float3 directionVector : POSITION;
};

[[vk::binding(0, 0)]] SamplerState uSkyboxSampler;
[[vk::binding(0, 0)]] TextureCube uSkyboxTexture;

float4 main(PSInput input) : SV_Target
{
    float3 dir = normalize(input.directionVector);
    dir = float3(dir.x, dir.y, -dir.z); // For flipping front and back
    return uSkyboxTexture.Sample(uSkyboxSampler, dir);
}