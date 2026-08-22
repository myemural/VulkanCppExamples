#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

struct WindCell
{
    vec4 dirStrength; // xy = Wind direction (xz-plane, normalized), z = Strength, w = Unused
};

struct LineVertex
{
    vec4 position;
    vec4 color;
};

layout(std140, binding = 0) uniform WindUbo
{
    vec4 volumeMin;        // xyz = Volume min corner position
    vec4 volumeMax;        // xyz = Volume max corner position
    vec4 windDirTimeAlpha; // xy = Base wind direction (xz), z = Time (seconds), w  = Unused
    vec4 params;           // x = Base strength, y = Gust speed, z = Noise scale, w = Grid size (float)
} wind;

layout(std430, binding = 1) writeonly buffer WindCellBuffer
{
    WindCell cells[];
};

layout(std430, binding = 2) writeonly buffer WindDebugLineBuffer
{
    LineVertex lines[];
};

const float ARROW_LENGTH_SCALE = 0.6; // Purely cosmetic, only affects the debug arrow size

// Pseudo-random generator function for float between [0, 1)
float hash1(vec2 seed)
{
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Simple bilinear value-noise, this is intentionally not Simplex/Perlin noise, because it is cheap
float valueNoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash1(i);
    float b = hash1(i + vec2(1.0, 0.0));
    float c = hash1(i + vec2(0.0, 1.0));
    float d = hash1(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

vec2 rotate2D(vec2 v, float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    return vec2(v.x * c - v.y * s, v.x * s + v.y * c);
}

void main()
{
    ivec2 gridSize = ivec2(int(wind.params.w));
    ivec2 cellId = ivec2(gl_GlobalInvocationID.xy);
    if (cellId.x >= gridSize.x || cellId.y >= gridSize.y) {
        return;
    }

    int index = cellId.y * gridSize.x + cellId.x;

    // World-space center of this cell, only used to seed the noise so gusts drift smoothly across the volume
    vec2 cellUv = (vec2(cellId) + 0.5) / vec2(gridSize);
    vec3 cellWorldPos = mix(wind.volumeMin.xyz, wind.volumeMax.xyz, vec3(cellUv.x, 0.0, cellUv.y));

    float time = wind.windDirTimeAlpha.z;
    float noiseScale = wind.params.z;
    float gustSpeed = wind.params.y;

    // Time drifts the sample point through the noise field, giving each cell a slow, organic gust cycle
    vec2 noiseCoord = cellWorldPos.xz * noiseScale + vec2(time * gustSpeed, -time * gustSpeed * 0.6);

    float gustNoise = valueNoise(noiseCoord);
    float angleNoise = valueNoise(noiseCoord + vec2(19.7, 5.3));

    float gust = 0.55 + 0.45 * gustNoise; // Strength multiplier, stays positive, no dead cells
    float angleJitter = (angleNoise - 0.5) * 1.0; // +-0.5 rad direction wobble around the base wind

    vec2 baseDir = normalize(wind.windDirTimeAlpha.xy);
    vec2 finalDir = normalize(rotate2D(baseDir, angleJitter));
    float finalStrength = wind.params.x * gust;

    cells[index].dirStrength = vec4(finalDir, finalStrength, 0.0);

    // Debug arrow: One line per cell, lifted slightly above the ground, pointing along the wind direction
    vec3 arrowStart = cellWorldPos + vec3(0.0, 0.05, 0.0);
    vec3 arrowEnd = arrowStart + vec3(finalDir.x, 0.0, finalDir.y) * finalStrength * ARROW_LENGTH_SCALE;
    vec3 arrowColor = mix(vec3(0.2, 0.6, 1.0), vec3(1.0, 0.25, 0.2), clamp(finalStrength, 0.0, 1.0));

    lines[index * 2 + 0].position = vec4(arrowStart, 1.0);
    lines[index * 2 + 0].color = vec4(arrowColor, 1.0);
    lines[index * 2 + 1].position = vec4(arrowEnd, 1.0);
    lines[index * 2 + 1].color = vec4(arrowColor, 1.0);
}
