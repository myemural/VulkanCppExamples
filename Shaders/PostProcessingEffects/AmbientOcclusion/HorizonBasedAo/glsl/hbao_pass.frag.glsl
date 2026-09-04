#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define TWO_PI 6.28318530718

layout(location = 0) out float outOcclusion;

layout(set = 0, binding = 0) uniform sampler2D gPosition;
layout(set = 0, binding = 1) uniform sampler2D gNormal;
layout(set = 0, binding = 2) uniform sampler2D uNoise;

layout(push_constant) uniform SsaoPushConstants {
    mat4 projection;
    float radius;
    float angleBias;
    float power;
    uint directionCount;
    uint stepCount;
} pc;

// Every elevation angle is stored as its tangent, because it can be calculated without any trigonometry
float tangentOfSample(vec3 delta, float horizontalDistance)
{
    // The camera looks down th -Z axis, so a greater Z value means a point closer to the camera
    return delta.z / horizontalDistance;
}

float tangentToSine(float tangentValue)
{
    return tangentValue * inversesqrt(1.0 + tangentValue * tangentValue);
}

void main()
{
    // Maximum march length in pixels, keeps the texture reads local when the surface is close to the camera
    const float maxRadiusInPixels = 96.0;

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

    // Project the view-space radius into UV-space,
    vec2 uvRadius = 0.5 * pc.radius * vec2(pc.projection[0][0], abs(pc.projection[1][1])) / max(-fragPosView.z, 0.0001);

    vec2 radiusInPixels = uvRadius * screenSize;
    float longestSide = max(radiusInPixels.x, radiusInPixels.y);

    // Nothing to gather if the projected radius is smaller than a pixel
    if (longestSide < 1.0) {
        outOcclusion = 1.0;
        return;
    }

    if (longestSide > maxRadiusInPixels) {
        uvRadius *= maxRadiusInPixels / longestSide;
    }

    // Tile the noise texture over the screen to get a per-pixel rotation and step offset
    vec2 noiseScale = screenSize / vec2(textureSize(uNoise, 0));
    vec2 randomValues = texture(uNoise, uv * noiseScale).xy;

    float angleOffset = randomValues.x * TWO_PI / float(pc.directionCount);
    float stepOffset = randomValues.y;

    float occlusion = 0.0;
    for (uint d = 0; d < pc.directionCount; ++d) {
        float angle = angleOffset + TWO_PI * float(d) / float(pc.directionCount);

        // Marching direction in UV-space and the same direction in view-space (UV: +V points down, View: +Y points up)
        vec2 directionUv = vec2(cos(angle), sin(angle));
        vec3 directionView = normalize(vec3(directionUv.x, -directionUv.y, 0.0));

        // The horizon starts at the tangent plane of the surface, biased to reject nearly coplanar samples
        vec3 tangentVector = directionView - normalView * dot(directionView, normalView);
        float tangentHorizontal = length(tangentVector.xy);

        float tangentOfHorizon = tangentHorizontal > 0.0001
                                    ? tan(atan(tangentVector.z / tangentHorizontal) + pc.angleBias) : 0.0;
        float sineOfHorizon = tangentToSine(tangentOfHorizon);

        vec2 stepUv = (uvRadius * directionUv) / float(pc.stepCount);

        for (uint s = 0; s < pc.stepCount; ++s) {
            vec2 sampleUv = uv + stepUv * (float(s) + stepOffset + 0.5);

            vec4 neighbourSample = texture(gPosition, sampleUv);
            if (neighbourSample.w < 0.5) {
                continue;
            }

            vec3 delta = neighbourSample.xyz - fragPosView;
            float horizontalDistance = length(delta.xy);
            if (horizontalDistance < 0.0001) {
                continue;
            }

            // A sample only contributes while it raises the horizon of the current direction
            float tangentOfSampleValue = tangentOfSample(delta, horizontalDistance);
            if (tangentOfSampleValue <= tangentOfHorizon) {
                continue;
            }

            float distanceSquared = dot(delta, delta);
            float falloff = clamp(1.0 - distanceSquared / (pc.radius * pc.radius), 0.0, 1.0);

            float sineOfSample = tangentToSine(tangentOfSampleValue);
            occlusion += falloff * (sineOfSample - sineOfHorizon);

            tangentOfHorizon = tangentOfSampleValue;
            sineOfHorizon = sineOfSample;
        }
    }

    occlusion = 1.0 - (occlusion / float(pc.directionCount));

    outOcclusion = pow(clamp(occlusion, 0.0, 1.0), pc.power);
}
