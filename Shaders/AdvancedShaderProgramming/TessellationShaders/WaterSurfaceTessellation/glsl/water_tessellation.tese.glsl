#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define TWO_PI 6.28318530718
#define NUM_WAVES 4

layout(quads, fractional_odd_spacing, ccw) in;

layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec2 inUv[];
layout(location = 2) in vec3 inNormal[];

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec2 fragUv;
layout(location = 2) out mat3 fragTBN;
layout(location = 5) out float fragWaveHeight; // Signed wave displacement

struct MeshTransformData
{
    mat4 model;
    mat4 normalMatrix;
};
layout(std430, binding = 0) readonly buffer MeshTransformDataBuffer {
    MeshTransformData meshTransforms[];
};

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
    float tessellationLevel;
    float displacementLevel;
    float time;
} pc;

// ------------------------------------------------------------------------
// Gerstner Wave Model
// ------------------------------------------------------------------------
// Each wave moves points on the surface along an elliptical (trochoidal) path instead of just pushing them straight up,
// which is what gives real water its characteristic sharp crests and broad troughs. Summing a few waves of different
// direction/wavelength/speed avoids an obviously repeating, "sloshing pool" look
struct GerstnerWave
{
    vec2 direction;  // Travel direction on the XZ plane (need not be normalized)
    float steepness; // 0-1, how peaked the crest is
    float wavelength;
    float speed;     // World units per second the wave crest travels
};

const GerstnerWave waves[NUM_WAVES] = GerstnerWave[NUM_WAVES](
        GerstnerWave(vec2(1.0, 0.0),  0.30, 12.0, 1.2),
        GerstnerWave(vec2(0.7, 0.7),  0.22,  7.0, 1.6),
        GerstnerWave(vec2(-0.3, 0.9), 0.18,  4.5, 2.1),
        GerstnerWave(vec2(0.6, -0.5), 0.12,  2.5, 3.0)
);

// Accumulates one wave's contribution to position offset and to the two tangent (partial derivative) vectors used to
// build the analytic normal
void applyGerstnerWave(GerstnerWave w, vec2 posXZ, float t, float amplitudeScale,
                       inout vec3 offset, inout vec3 dPdx, inout vec3 dPdz)
{
    vec2 d = normalize(w.direction);
    float k = TWO_PI / w.wavelength;
    float a = amplitudeScale * (w.steepness / k);
    float phase = k * w.speed;
    float theta = k * dot(d, posXZ) - phase * t;

    float s = sin(theta);
    float c = cos(theta);

    offset.x += d.x * a * c;
    offset.y += a * s;
    offset.z += d.y * a * c;

    float qk = w.steepness * a * k;

    dPdx.x += -d.x * d.x * qk * s;
    dPdx.y += d.x * a * k * c;
    dPdx.z += -d.x * d.y * qk * s;

    dPdz.x += -d.x * d.y * qk * s;
    dPdz.y += d.y * a * k * c;
    dPdz.z += -d.y * d.y * qk * s;
}

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2, vec2 v3)
{
    vec2 bottom = mix(v0, v1, gl_TessCoord.x);
    vec2 top = mix(v3, v2, gl_TessCoord.x);
    return mix(bottom, top, gl_TessCoord.y);
}

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2, vec3 v3)
{
    vec3 bottom = mix(v0, v1, gl_TessCoord.x);
    vec3 top = mix(v3, v2, gl_TessCoord.x);
    return mix(bottom, top, gl_TessCoord.y);
}

vec4 interpolate4D(vec4 v0, vec4 v1, vec4 v2, vec4 v3)
{
    vec4 bottom = mix(v0, v1, gl_TessCoord.x);
    vec4 top = mix(v3, v2, gl_TessCoord.x);
    return mix(bottom, top, gl_TessCoord.y);
}

void main()
{
    const MeshTransformData meshTransform = meshTransforms[pc.objectId];

    // Object-space bilinear interpolation across the quad patch
    vec3 position = interpolate3D(inPosition[0], inPosition[1], inPosition[2], inPosition[3]);
    vec2 uv = interpolate2D(inUv[0], inUv[1], inUv[2], inUv[3]);

    // Undisplaced world-space position. We evaluate the waves against world coordinates (not object-space ones)
    // so that wavelength/speed are real world units, independent of whatever scale the object happens to have
    vec4 worldPos0 = meshTransform.model * vec4(position, 1.0);

    vec3 offset = vec3(0.0);
    vec3 dPdx = vec3(1.0, 0.0, 0.0);
    vec3 dPdz = vec3(0.0, 0.0, 1.0);

    for (int i = 0; i < NUM_WAVES; ++i) {
        applyGerstnerWave(waves[i], worldPos0.xz, pc.time, pc.displacementLevel, offset, dPdx, dPdz);
    }

    vec4 worldPos = worldPos0 + vec4(offset, 0.0);

    // Analytic normal/tangent from the wave partial derivatives. These are already expressed in world-space,
    // no further normalMatrix multiplication is needed here
    vec3 N = normalize(cross(dPdz, dPdx));
    vec3 T = normalize(dPdx - N * dot(N, dPdx));
    vec3 B = cross(N, T);

    fragPos = worldPos.xyz;
    fragUv = uv;
    fragWaveHeight = offset.y;
    fragTBN = mat3(T, B, N);

    gl_Position = pc.proj * pc.view * worldPos;
}
