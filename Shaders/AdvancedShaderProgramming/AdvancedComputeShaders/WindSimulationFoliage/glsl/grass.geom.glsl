#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

#define TWO_PI 6.28318530718

layout (points) in;
layout (triangle_strip, max_vertices = 40) out; // NUM_BLADES × (NUM_SEGMENTS + 1) × 2

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragWorldPos;
layout(location = 2) out float fragHeightT;          // 0 = root, 1 = tip
layout(location = 3) out float fragSide;             // -1 = left edge, 1 = right edge
layout(location = 4) flat out float fragBladeRandom; // Per-blade random value for color variance

struct WindCell
{
    vec4 dirStrength; // xy = Wind direction (xz-plane, normalized), z = Strength, w = Unused
};

layout(std140, binding = 4) uniform WindUbo {
    vec4 volumeMin;        // xyz = Volume min corner position
    vec4 volumeMax;        // xyz = Volume max corner position
    vec4 windDirTimeAlpha; // xy = Base wind direction (xz), z = Time (seconds), w  = Unused
    vec4 params;           // x = Base strength, y = Gust speed, z = Noise scale, w = Grid size (float)
} wind;

layout(std430, binding = 5) readonly buffer WindCellBuffer {
    WindCell windCells[];
};

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

const int NUM_BLADES = 5;          // Blade count per input point
const int NUM_SEGMENTS = 3;        // Quad segment count per blade (SEGMENTS+1 cross sections)
const float BASE_HEIGHT = 0.6;     // World-space height of the one blade
const float HEIGHT_VARIANCE = 0.4; // Variance of the BASE_HEIGHT
const float BASE_WIDTH = 0.05;     // World-space width of the one blade's root
const float WIDTH_VARIANCE = 0.4;  // Variance of the BASE_WIDTH
const float MIN_BEND = 0.15;       // Minimum horizontal lean at the tip
const float MAX_BEND = 0.55;       // Maximum horizontal lean at the tip
const float CLUMP_RADIUS = 0.18;   // Scatter radius around the input point

// Pseudo-random generator function for float between [0, 1)
float hash1(vec2 seed)
{
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Pseudo-random generator function for vec2 between [0, 1)
vec2 hash2(vec2 seed)
{
    return vec2(hash1(seed), hash1(seed + vec2(37.1, 91.7)));
}

// Samples the wind grid at a world-space position, returns a vector already scaled by strength (xz-plane, y = 0)
// Returns exactly (0,0,0) outside the wind volume, so grass outside the box is completely unaffected
vec3 sampleWind(vec3 worldPos)
{
    vec2 extent = wind.volumeMax.xz - wind.volumeMin.xz;
    vec2 uv = (worldPos.xz - wind.volumeMin.xz) / max(extent, vec2(0.0001));

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0 ||
        worldPos.y < wind.volumeMin.y || worldPos.y > wind.volumeMax.y) {
        return vec3(0.0);
    }

    int gridSize = int(wind.params.w);
    ivec2 cell = clamp(ivec2(uv * float(gridSize)), ivec2(0), ivec2(gridSize - 1));
    int index = cell.y * gridSize + cell.x;

    vec4 c = windCells[index].dirStrength;
    return vec3(c.x, 0.0, c.y) * c.z;
}

void emitVertex(vec3 pos, vec3 normal, float t, float side, float bladeRandomColor)
{
    fragNormal = normal;
    fragWorldPos = pos;
    fragHeightT = t;
    fragSide = side;
    fragBladeRandom = bladeRandomColor;
    gl_Position = pc.proj * pc.view * vec4(pos, 1.0);
    EmitVertex();
}

void main()
{
    // Incoming position should be in world-space
    vec3 basePos = gl_in[0].gl_Position.xyz;

    // Draw every blade around base point
    for (int bladeIndex = 0; bladeIndex < NUM_BLADES; ++bladeIndex)
    {
        // Generate seed value for random number generation
        vec2 seed = basePos.xz * 17.0 + vec2(float(bladeIndex) * 91.13, float(bladeIndex) * 7.77) + basePos.y * 3.0;

        // Generate some random values for grass position, size and direction values
        float randomAngle = hash1(seed);
        float randomHeight = hash1(seed + 3.1);
        float randomWidth = hash1(seed + 6.2);
        float randomBend = hash1(seed + 9.3);
        float randomBendDir = hash1(seed + 12.4);
        vec2 randomOffset = hash2(seed + 15.5) * 2.0 - 1.0; // Between [-1, 1]

        // Scatter the blade origin around the input point so a clump doesn't
        // look like every blade grows from the exact same spot.
        vec3 origin = basePos + vec3(randomOffset.x, 0.0, randomOffset.y) * CLUMP_RADIUS;

        // Calculate random height, root width and bend amound of the one blade
        float height = BASE_HEIGHT * (1.0 - HEIGHT_VARIANCE * 0.5 + randomHeight * HEIGHT_VARIANCE);
        float width = BASE_WIDTH  * (1.0 - WIDTH_VARIANCE  * 0.5 + randomWidth  * WIDTH_VARIANCE);
        float bend = mix(MIN_BEND, MAX_BEND, randomBend);

        float facing = randomAngle * TWO_PI; // Orientation of the blade's flat plane
        float leanAngle  = facing + (randomBendDir - 0.5) * 1.6; // Lean direction of the blade between -45 and 45 degrees
        vec3 widthDir = vec3(-sin(facing), 0.0, cos(facing)); // Width direction across the blade
        vec3 leanDir  = vec3(cos(leanAngle), 0.0, sin(leanAngle)); // Direction the blade bends toward
        float bladeRandomColor = randomAngle; // Random value for blade color (for fragment shader)

        // Wind contribution, sampled once per blade (not per segment) so the whole blade sways as one piece
        vec3 windVec = sampleWind(origin);
        float windStrength = length(windVec);
        vec3 windDirN = windStrength > 0.0001 ? windVec / windStrength : vec3(0.0);

        // A gentle per-blade flutter riding on top of the wind lean, phase offset by the
        // blade's own random seed so neighbouring blades don't move in lockstep
        float flutter = sin(wind.windDirTimeAlpha.z * 3.2 + randomAngle * TWO_PI) * 0.5 + 0.5;
        float windBend = windStrength * (0.12 + 0.18 * flutter);

        // Draw every segment of the blade
        for (int segmentIndex = 0; segmentIndex <= NUM_SEGMENTS; ++segmentIndex)
        {
            float t = float(segmentIndex) / float(NUM_SEGMENTS);

            // Ease-out curve, most of the bend happens near the tip, base stays upright
            float curveT = t * t;
            vec3 windOffset = windDirN * (windBend * curveT);
            vec3 center = origin + vec3(0.0, height * t, 0.0) + leanDir * (bend * curveT) + windOffset;

            // Taper width toward the tip, pinch to a sharp point at the last cross section
            float halfWidth = width * 0.5 * (1.0 - t);
            if (segmentIndex == NUM_SEGMENTS) {
                halfWidth = 0.0;
            }

            // Approximate the curve's tangent to build a usable lighting normal
            // Wind is folded in here too so the normal stays consistent with the bent shape
            vec3 tangent = normalize(vec3(0.0, height, 0.0) + leanDir * (bend * 2.0 * t) + windDirN * (windBend * 2.0 * t));
            vec3 normal = normalize(cross(widthDir, tangent));

            // Keep it pointing roughly upward/outward
            if (normal.y < 0.0) {
                normal = -normal;
            }

            vec3 left = center - widthDir * halfWidth;
            vec3 right = center + widthDir * halfWidth;

            emitVertex(left, normal, t, -1.0, bladeRandomColor);
            emitVertex(right, normal, t,  1.0, bladeRandomColor);
        }

        EndPrimitive();
    }
}