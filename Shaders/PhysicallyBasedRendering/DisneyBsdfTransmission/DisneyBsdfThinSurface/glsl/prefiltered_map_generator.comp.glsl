#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0) uniform samplerCube environmentMap;

layout(binding = 1, rgba32f) uniform imageCube outPrefilterMap;

layout(push_constant) uniform PushConstants
{
    float roughness;
    uint mipLevel;
    uint mipSize;
} pc;

const float PI = 3.14159265359;

// Reference: https://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness * roughness;

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

vec3 getCubeDir(uint face, vec2 uv)
{
    uv = uv * 2.0 - 1.0;

    vec3 dir;
    switch (face)
    {
        case 0u: // +X
            dir = vec3( 1.0, -uv.y, -uv.x);
            break;

        case 1u: // -X
            dir = vec3(-1.0, -uv.y,  uv.x);
            break;

        case 2u: // +Y
            dir = vec3( uv.x,  1.0,  uv.y);
            break;

        case 3u: // -Y
            dir = vec3( uv.x, -1.0, -uv.y);
            break;

        case 4u: // +Z
            dir = vec3( uv.x, -uv.y,  1.0);
            break;

        default: // -Z
            dir = vec3(-uv.x, -uv.y, -1.0);
            break;
    }

    dir.z = -dir.z; // For flipping front and back

    return normalize(dir);
}

void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

    uint face = gl_GlobalInvocationID.z;

    if (pixel.x >= int(pc.mipSize) || pixel.y >= int(pc.mipSize)) {
        return;
    }

    vec2 uv = (vec2(pixel) + 0.5) / float(pc.mipSize);
    vec3 N = getCubeDir(face, uv);
    vec3 V = N;

    const uint SAMPLE_COUNT = 1024u;
    vec3 prefiltered = vec3(0.0);
    float totalWeight = 0.0;
    for (uint i = 0u; i < SAMPLE_COUNT; i++)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = importanceSampleGGX(Xi, N, pc.roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0)
        {
            prefiltered += texture(environmentMap, L).rgb * NdotL;
            totalWeight += NdotL;
        }
    }

    prefiltered /= max(totalWeight, 0.0001);

    imageStore(outPrefilterMap, ivec3(pixel, int(face)), vec4(prefiltered, 1.0));
}