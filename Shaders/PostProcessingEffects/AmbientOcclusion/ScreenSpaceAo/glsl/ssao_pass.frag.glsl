#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out float outOcclusion;

layout(set = 0, binding = 0) uniform sampler2D gPosition;
layout(set = 0, binding = 1) uniform sampler2D gNormal;
layout(set = 0, binding = 2) uniform sampler2D uNoise;

const uint MAX_KERNEL_SIZE = 64;

layout(set = 0, binding = 3) uniform SsaoKenelBuffer
{
    vec4 samples[MAX_KERNEL_SIZE];
} ssaoKernel;

layout(push_constant) uniform SsaoPushConstants {
    mat4 projection;
    float radius;
    float bias;
    float power;
    uint kernelSize;
} pc;

void main()
{
    vec2 screenSize = vec2(textureSize(gPosition, 0));
    vec2 uv = gl_FragCoord.xy / screenSize;

    vec4 positionSample = texture(gPosition, uv);

    // Skip the background
    if (positionSample.w < 0.5) {
        outOcclusion = 1.0;
        return;
    }

    vec3 fragPosView = positionSample.xyz;
    vec3 normalView = normalize(texture(gNormal, uv).xyz);

    // Tile the noise texture over the screen to get a per-pixel rotation vector
    vec2 noiseScale = screenSize / vec2(textureSize(uNoise, 0));
    vec3 randomVec = normalize(vec3(texture(uNoise, uv * noiseScale).xy * 2.0 - 1.0, 0.0));

    // Gram-Schmidt to build a randomly rotated basis around the surface normal
    vec3 tangent = normalize(randomVec - normalView * dot(randomVec, normalView));
    vec3 bitangent = cross(normalView, tangent);
    mat3 tbn = mat3(tangent, bitangent, normalView);

    float occlusion = 0.0;
    for (uint i = 0; i < pc.kernelSize; ++i) {
        // Tangent space sample to view space
        vec3 samplePos = fragPosView + (tbn * ssaoKernel.samples[i].xyz * pc.radius);

        // Project the sample position to get its screen-space location
        vec4 offset = pc.projection * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        vec2 sampleUv = offset.xy * 0.5 + 0.5;

        // Geometry depth (view-space Z) stored at the sample's screen position
        float sampleDepth = texture(gPosition, sampleUv).z;

        // Only accept samples that are within the radius of the current fragment
        float rangeCheck = smoothstep(0.0, 1.0, pc.radius / abs(fragPosView.z - sampleDepth));

        // The camera looks down the -Z axis, so a greater Z value means closer to the camera
        occlusion += (sampleDepth >= samplePos.z + pc.bias ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / float(pc.kernelSize));

    outOcclusion = pow(clamp(occlusion, 0.0, 1.0), pc.power);
}
