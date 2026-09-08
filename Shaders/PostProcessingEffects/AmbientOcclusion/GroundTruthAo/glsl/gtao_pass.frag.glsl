#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define PI 3.14159265
#define HALF_PI 1.57079633

layout(location = 0) out float outOcclusion;

layout(set = 0, binding = 0) uniform sampler2D gPosition;
layout(set = 0, binding = 1) uniform sampler2D gNormal;

layout(push_constant) uniform SsaoPushConstants {
    mat4 projection;
    float radius;
    float thickness;
    float power;
    uint sliceCount;
    uint stepsPerSlice;
} pc;

// Fraction of the radius where the distance attenuation starts
const float kFalloffRange = 0.6;

// Maximum march length in pixels, keeps the texture reads local when the surface is close to the camera
const float kMaxRadiusInPixels = 96.0;

// The 4x4 spatial jitter pattern, it matches the 4x4 box blur of the next pass
void spatialJitter(uvec2 pixelCoord, out float sliceJitter, out float stepJitter)
{
    sliceJitter = float((((pixelCoord.x + pixelCoord.y) & 0x3U) << 2U) + (pixelCoord.x & 0x3U)) / 16.0;
    stepJitter = float((pixelCoord.x + pixelCoord.y) & 0x3U) / 4.0;
}

// Cosine-weighted visibility of the single slice, solved analytically over the arc between the two horizons
float sliceVisibility(float normalAngle, float horizonAngle0, float horizonAngle1)
{
    float sineOfNormal = sin(normalAngle);
    float cosineOfNormal = cos(normalAngle);

    float arc0 = 2.0 * horizonAngle0 * sineOfNormal - cos(2.0 * horizonAngle0 - normalAngle) + cosineOfNormal;
    float arc1 = 2.0 * horizonAngle1 * sineOfNormal - cos(2.0 * horizonAngle1 - normalAngle) + cosineOfNormal;

    return 0.25 * (arc0 + arc1);
}

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
    vec3 viewVec = normalize(-fragPosView);

    // UV offet that corresponds to a unit length view-space vector on the XY plane
    vec2 projScale = 0.5 * vec2(pc.projection[0][0], -abs(pc.projection[1][1])) / max(-fragPosView.z, 0.0001);

    // Shorten the radius when its projection would cover too many pixels
    float pixelsPerUnit = length(projScale * screenSize);
    if (pixelsPerUnit < 0.0001) {
        outOcclusion = 1.0;
        return;
    }

    float radius = min(pc.radius, kMaxRadiusInPixels / pixelsPerUnit);
    float falloffStart = radius * (1.0 - kFalloffRange);
    float falloffScale = 1.0 / max(radius - falloffStart, 0.0001);

    float sliceJitter;
    float stepJitter;
    spatialJitter(uvec2(gl_FragCoord.xy), sliceJitter, stepJitter);

    float visibility = 0.0;
    for (uint slice = 0; slice < pc.sliceCount; ++slice) {
        // Every slice is searched in both directions, so the slices only need to cover half a rotation
        float phi = PI * (float(slice) * sliceJitter) / float(pc.sliceCount);

        // Marching direction on the view-space XY plane and the matching UV offset per unit length
        vec3 directionVec = vec3(cos(phi), sin(phi), 0.0);
        vec2 uvPerUnit = projScale * directionVec.xy;

        // Orthonormal frame of the slice, the plane that contains the view vector and the marchign direction
        vec3 orthoDirectionVec = directionVec - dot(directionVec, viewVec) * viewVec;
        vec3 axisVec = normalize(cross(orthoDirectionVec, viewVec));

        // Project the surface normal onto the slice plane, its length is the weight of this slice
        vec3 projectedNormalVec = normalView - axisVec * dot(normalView, axisVec);
        float projectedNormalLength = length(projectedNormalVec);

        // The slice is perpendicular to the normal, its cosine weight is zero
        if (projectedNormalLength < 0.0001) {
            continue;
        }

        // Signed angle between the projected normal and view vector, measured inside the slice plane
        float cosineOfNormal = clamp(dot(projectedNormalVec, viewVec) / projectedNormalLength, -1.0, 1.0);
        float normalAngle = sign(dot(orthoDirectionVec, projectedNormalVec)) * acos(cosineOfNormal);

        // Both horizons start on the tangent plane of the projected normal
        float lowHorizonCosine0 = cos(normalAngle + HALF_PI);
        float lowHorizonCosine1 = cos(normalAngle - HALF_PI);
        float horizonCosine0 = lowHorizonCosine0;
        float horizonCosine1 = lowHorizonCosine1;

        // Never sample the texel of the fragment itself
        float minDistance = 1.5 / length(uvPerUnit * screenSize);

        for (uint step = 0; step < pc.stepsPerSlice; ++step) {
            float stepFraction = (float(step) + stepJitter) / float(pc.stepsPerSlice);

            // Squared distribution puts more samples into the near field, where the occlusion detail is
            stepFraction *= stepFraction;

            float sampleDistance = max(stepFraction * radius, minDistance);
            vec2 sampleUvOffset = uvPerUnit * sampleDistance;

            // Positive side of the slice
            vec4 neighbourSample0 = texture(gPosition, uv + sampleUvOffset);
            if (neighbourSample0.w > 0.5) {
                vec3 delta = neighbourSample0.xyz -fragPosView;
                float distance = length(delta);

                if (distance > 0.0001) {
                    float sampleCosine = dot(delta / distance, viewVec);

                    // Fade the sample cut towards the edge of the radius
                    float weight = clamp((radius - distance) * falloffScale, 0.0, 1.0);
                    sampleCosine = mix(lowHorizonCosine0, sampleCosine, weight);

                    // A lower sample is allowed to pull the horizon back down
                    horizonCosine0 = sampleCosine > horizonCosine0 ?
                                     sampleCosine : mix(sampleCosine, horizonCosine0, pc.thickness);
                }
            }

            // Negative side of the slice
            vec4 neighbourSample1 = texture(gPosition, uv - sampleUvOffset);
            if (neighbourSample1.w > 0.5) {
                vec3 delta = neighbourSample1.xyz -fragPosView;
                float distance = length(delta);

                if (distance > 0.0001) {
                    float sampleCosine = dot(delta / distance, viewVec);

                    float weight = clamp((radius - distance) * falloffScale, 0.0, 1.0);
                    sampleCosine = mix(lowHorizonCosine1, sampleCosine, weight);

                    horizonCosine1 = sampleCosine > horizonCosine1 ?
                                     sampleCosine : mix(sampleCosine, horizonCosine1, pc.thickness);
                }
            }
        }

        // Back to angles and clamp both horizons into the hemisphere of the projected normal
        float horizonAngle0 = -acos(clamp(horizonCosine1, -1.0, 1.0));
        float horizonAngle1 = acos(clamp(horizonCosine0, -1.0, 1.0));

        horizonAngle0 = normalAngle + max(horizonAngle0 - normalAngle, -HALF_PI);
        horizonAngle1 = normalAngle + min(horizonAngle1 - normalAngle, HALF_PI);

        visibility += projectedNormalLength * sliceVisibility(normalAngle, horizonAngle0, horizonAngle1);
    }

    visibility /= float(pc.sliceCount);

    outOcclusion = pow(clamp(visibility, 0.0, 1.0), pc.power);
}
