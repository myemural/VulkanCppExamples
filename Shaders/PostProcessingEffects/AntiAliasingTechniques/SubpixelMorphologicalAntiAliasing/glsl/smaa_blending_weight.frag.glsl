#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outWeights;

layout(set = 0, binding = 0) uniform sampler2D gEdges;     // Output of the edge detection (linear filtering)
layout(set = 0, binding = 1) uniform sampler2D gAreaTex;   // Precomputed areas (linear filtering)
layout(set = 0, binding = 2) uniform sampler2D gSearchTex; // Precomputed search lengths (point filtering)

layout(push_constant) uniform SmaaPushConstants {
    float threshold;               // Luma difference threshold of the edges
    uint maxSearchSteps;           // Search steps of the horizontal/vertical lines
    uint maxSearchStepsDiag;       // Search steps of the diagonal lines (0: Diagonal detection disabled)
    uint isCornerDetectionEnabled; // 0: Sharp corners are blended, 1: Sharp corners are preserved
    uint isSmaaEnabled;            // 0: SMAA disabled, 1: SMAA enabled
    uint debugView;                // 0: Final image, 1: Edges, 2: Blending weights
} pc;

// Layout of the precomputed textures
const float kAreaTexMaxDistance = 16.0;
const float kAreaTexMaxDistanceDiag = 20.0;
const vec2 kAreaTexPixelSize = 1.0 / vec2(160.0, 560.0);
const float kAreaTexSubtexSize = 1.0 / 7.0;
const vec2 kSearchTexSize = vec2(66.0, 33.0);
const vec2 kSearchTexPackedSize = vec2(64.0, 16.0);

// Amount of the blending at sharp corners (0: Sharp corners, 1: Fully blended)
const float kCornerRounding = 0.25;

// 1x: No sub-sample offsets (used by temporal / multi-sample modes of SMAA)
const vec4 kSubsampleIndices = vec4(0.0);

vec4 gRtMetrics; // xy = 1 / resolution, zw = resolution

vec4 sampleEdges(vec2 uv)
{
    return textureLod(gEdges, uv, 0.0);
}

// ------------ Diagonals ------------

// Bilinear fetches with 0.25 offset: R edge gives 0.25 or 1.0, G edge gives 0.75 or 1.0, onpack to 0 or 1
vec2 decodeDiagBilinearAccess(vec2 e)
{
    e.r = e.r * abs(5.0 * e.r - 5.0 * 0.75);
    return round(e);
}

vec4 decodeDiagBilinearAccess(vec4 e)
{
    e.rb = e.rb * abs(5.0 * e.rb - 5.0 * 0.75);
    return round(e);
}

vec2 searchDiag1(vec2 uv, vec2 direction, out vec2 e)
{
    vec4 coord = vec4(uv, -1.0, 1.0);
    vec3 t = vec3(gRtMetrics.xy, 1.0);
    e = vec2(0.0);
    while (coord.z < float(pc.maxSearchStepsDiag) - 1.0 && coord.w > 0.9) {
        coord.xyz = t * vec3(direction, 1.0) + coord.xyz;
        e = sampleEdges(coord.xy).rg;
        coord.w = dot(e, vec2(0.5));
    }

    return coord.zw;
}

vec2 searchDiag2(vec2 uv, vec2 direction, out vec2 e)
{
    vec4 coord = vec4(uv, -1.0, 1.0);
    coord.x += 0.25 * gRtMetrics.x; // Both edges are fetched at once with bilinear filtering
    vec3 t = vec3(gRtMetrics.xy, 1.0);
    e = vec2(0.0);
    while (coord.z < float(pc.maxSearchStepsDiag) - 1.0 && coord.w > 0.9) {
        coord.xyz = t * vec3(direction, 1.0) + coord.xyz;
        e = decodeDiagBilinearAccess(sampleEdges(coord.xy).rg);
        coord.w = dot(e, vec2(0.5));
    }

    return coord.zw;
}

vec2 areaDiag(vec2 distance, vec2 e, float offset)
{
    vec2 uv = vec2(kAreaTexMaxDistanceDiag) * e + distance;
    uv = kAreaTexPixelSize * uv + 0.5 * kAreaTexPixelSize; // Texel centers
    uv.x += 0.5; // Diagonal areas are on the right half of the texture
    uv.y += kAreaTexSubtexSize * offset;

    return textureLod(gAreaTex, uv, 0.0).rg;
}

vec2 calculateDiagWeights(vec2 uv, vec2 e)
{
    vec2 weights = vec2(0.0);
    vec4 d;
    vec2 end;

    // Diagonal going to bottom-left / top-right
    if (e.r > 0.0) {
        d.xz = searchDiag1(uv, vec2(-1.0, 1.0), end);
        d.x += float(end.y > 0.9);
    } else {
        d.xz = vec2(0.0);
    }
    d.yw = searchDiag1(uv, vec2(1.0, -1.0), end);

    // Line is longer than 3 pixels
    if (d.x + d.y > 2.0) {
        // Crossing edges at both ends
        vec4 coords = vec4(-d.x + 0.25, d.x, d.y, -d.y - 0.25) * gRtMetrics.xyxy + uv.xyxy;
        vec4 c;
        c.xy = textureLodOffset(gEdges, coords.xy, 0.0, ivec2(-1, 0)).rg;
        c.zw = textureLodOffset(gEdges, coords.zw, 0.0, ivec2(1, 0)).rg;
        c.yxwz = decodeDiagBilinearAccess(c.xyzw);

        vec2 cc = vec2(2.0) * c.xz + c.yw;

        // Crossing edge is removed if the end of the line is not found
        if (d.z >= 0.9) {
            cc.x = 0.0;
        }
        if (d.w >= 0.9) {
            cc.y = 0.0;
        }

        weights += areaDiag(d.xy, cc, kSubsampleIndices.z);
    }

    // Diagonal going to top-left / bottom-right
    d.xz = searchDiag2(uv, vec2(-1.0, -1.0), end);
    if (textureLodOffset(gEdges, uv, 0.0, ivec2(1, 0)).r > 0.0) {
        d.yw = searchDiag2(uv, vec2(1.0, 1.0), end);
        d.y += float(end.y > 0.9);
    } else {
        d.yw = vec2(0.0);
    }

    if (d.x + d.y > 2.0) {
        vec4 coords = vec4(-d.x, -d.x, d.y, d.y) * gRtMetrics.xyxy + uv.xyxy;
        vec4 c;
        c.x = textureLodOffset(gEdges, coords.xy, 0.0, ivec2(-1, 0)).g;
        c.y = textureLodOffset(gEdges, coords.xy, 0.0, ivec2(0, -1)).r;
        c.zw = textureLodOffset(gEdges, coords.zw, 0.0, ivec2(1, 0)).gr;

        vec2 cc = vec2(2.0) * c.xz + c.yw;

        if (d.z >= 0.9) {
            cc.x = 0.0;
        }
        if (d.w >= 0.9) {
            cc.y = 0.0;
        }

        weights += areaDiag(d.xy, cc, kSubsampleIndices.w).gr;
    }

    return weights;
}

// ------------ Horizontal / Vertical ------------

// The search texture gives the exact distance to the end of the line from the last bilinear fetch (four edges)
float searchLength(vec2 e, float offset)
{
    // The texture is flipped vertically, left and right cases take half of the space horizontally
    vec2 scale = kSearchTexSize * vec2(0.5, -1.0);
    vec2 bias = kSearchTexSize * vec2(offset, 1.0);

    // Scale and bias to access the texel centers
    scale += vec2(-1.0, 1.0);
    bias += vec2(0.5, -0.5);

    // Pixel coordinates to texture coordinates (the texture is cropped)
    scale *= 1.0 / kSearchTexPackedSize;
    bias *= 1.0 / kSearchTexPackedSize;

    return  textureLod(gSearchTex, scale * e + bias, 0.0).r;
}

// The coordinates offset by (-0.25, -0.125), so each bilinear fetch reads 4 edges (2 pixels)
// The different offsets in each difrection tell which edges are active
float searchXLeft(vec2 uv, float end)
{
    vec2 e = vec2(0.0, 1.0);

    // Line continues and no crossing edge breaks it
    while (uv.x > end && e.g > 0.8281 && e.r == 0.0) {
        e = sampleEdges(uv).rg;
        uv = -vec2(2.0, 0.0) * gRtMetrics.xy + uv;
    }

    float offset = -(255.0 / 127.0) * searchLength(e, 0.0) + 3.25;
    return gRtMetrics.x * offset + uv.x;
}

float searchXRight(vec2 uv, float end)
{
    vec2 e = vec2(0.0, 1.0);
    while (uv.x < end && e.g > 0.8281 && e.r == 0.0) {
        e = sampleEdges(uv).rg;
        uv = vec2(2.0, 0.0) * gRtMetrics.xy + uv;
    }

    float offset = -(255.0 / 127.0) * searchLength(e, 0.5) + 3.25;
    return -gRtMetrics.x * offset + uv.x;
}

float searchYUp(vec2 uv, float end)
{
    vec2 e = vec2(1.0, 0.0);
    while (uv.y > end && e.r > 0.8281 && e.g == 0.0) {
        e = sampleEdges(uv).rg;
        uv = -vec2(0.0, 2.0) * gRtMetrics.xy + uv;
    }

    float offset = -(255.0 / 127.0) * searchLength(e.gr, 0.0) + 3.25;
    return gRtMetrics.y * offset + uv.y;
}

float searchYDown(vec2 uv, float end)
{
    vec2 e = vec2(1.0, 0.0);
    while (uv.y < end && e.r > 0.8281 && e.g == 0.0) {
        e = sampleEdges(uv).rg;
        uv = vec2(0.0, 2.0) * gRtMetrics.xy + uv;
    }

    float offset = -(255.0 / 127.0) * searchLength(e.gr, 0.5) + 3.25;
    return -gRtMetrics.y * offset + uv.y;
}

// Area of the pattern: Sqrt of distances to both ends and the crossing edges at both sides
vec2 area(vec2 distance, float e1, float e2, float offset)
{
    // Rounding prevents precision errors of the bilinear filtering
    vec2 uv = vec2(kAreaTexMaxDistance) * round(4.0 * vec2(e1, e2)) + distance;
    uv = kAreaTexPixelSize * uv + 0.5 * kAreaTexPixelSize;
    uv.y = kAreaTexSubtexSize * offset + uv.y;

    return textureLod(gAreaTex, uv, 0.0).rg;
}

// ------------ Corners ------------

// Sharp corners (an edge continues perpendicularly at the end of the line) are blended less
void detectHorizontalCornerPattern(inout vec2 weights, vec4 uv, vec2 d)
{
    vec2 leftRight = step(d.xy, d.yx);
    vec2 rounding = (1.0 - kCornerRounding) * leftRight;
    rounding /= leftRight.x + leftRight.y; // Less blending for the pixels in the center of a line

    vec2 factor = vec2(1.0);
    factor.x -= rounding.x * textureLodOffset(gEdges, uv.xy, 0.0, ivec2(0, 1)).r;
    factor.x -= rounding.y * textureLodOffset(gEdges, uv.zw, 0.0, ivec2(1, 1)).r;
    factor.y -= rounding.x * textureLodOffset(gEdges, uv.xy, 0.0, ivec2(0, -2)).r;
    factor.y -= rounding.y * textureLodOffset(gEdges, uv.zw, 0.0, ivec2(1, -2)).r;

    weights *= clamp(factor, 0.0, 1.0);
}

void detectVerticalCornerPattern(inout vec2 weights, vec4 uv, vec2 d)
{
    vec2 leftRight = step(d.xy, d.yx);
    vec2 rounding = (1.0 - kCornerRounding) * leftRight;
    rounding /= leftRight.x + leftRight.y;

    vec2 factor = vec2(1.0);
    factor.x -= rounding.x * textureLodOffset(gEdges, uv.xy, 0.0, ivec2(1, 0)).g;
    factor.x -= rounding.y * textureLodOffset(gEdges, uv.zw, 0.0, ivec2(1, 1)).g;
    factor.y -= rounding.x * textureLodOffset(gEdges, uv.xy, 0.0, ivec2(-2, 0)).g;
    factor.y -= rounding.y * textureLodOffset(gEdges, uv.zw, 0.0, ivec2(-2, 1)).g;

    weights *= clamp(factor, 0.0, 1.0);
}

void main()
{
    vec2 resolution = vec2(textureSize(gEdges, 0));
    gRtMetrics = vec4(1.0 / resolution, resolution);

    vec2 pixelCoord = gl_FragCoord.xy;
    vec2 uv = pixelCoord * gRtMetrics.xy;

    // Search start coordinates (between the edges) and the search limits
    vec4 offset0 = gRtMetrics.xyxy * vec4(-0.25, -0.125, 1.25, -0.125) + uv.xyxy;
    vec4 offset1 = gRtMetrics.xyxy * vec4(-0.125, -0.25, -0.125, 1.25) + uv.xyxy;
    vec4 offset2 = gRtMetrics.xxyy * vec4(-2.0, 2.0, -2.0, 2.0) * float(pc.maxSearchSteps) +
                   vec4(offset0.xz, offset1.yw);

    vec4 weights = vec4(0.0);
    vec2 e = sampleEdges(uv).rg;

    // Edge at the top
    if (e.g > 0.0) {
        // Diagonals have both top and left edges, searching them at the top edges is enough
        bool isDiagonalFound = false;
        if (pc.maxSearchStepsDiag > 0U) {
            weights.rg = calculateDiagWeights(uv, e);
            isDiagonalFound = weights.r != -weights.g; // weights.r + weights.g != 0
        }

        // Diagonals have the priority, horizontal/vertical processing is skipped if a diagonal is found
        if (!isDiagonalFound) {
            vec2 d;

            // Distance to the left and the crossing edges there (sampled at -0.25 to read both edges)
            vec3 coords;
            coords.x = searchXLeft(offset0.xy, offset2.x);
            coords.y = offset1.y;
            d.x = coords.x;
            float e1 = sampleEdges(coords.xy).r;

            // Distance to the right end
            coords.z = searchXRight(offset0.zw, offset2.y);
            d.y = coords.z;

            // Distances in pixels, the area texture is compresses quadratically (sqrt of the distances)
            d = abs(round(gRtMetrics.zz * d - pixelCoord.xx));
            vec2 sqrtD = sqrt(d);

            float e2 = textureLodOffset(gEdges, coords.zy, 0.0, ivec2(1, 0)).r;

            weights.rg = area(sqrtD, e1, e2, kSubsampleIndices.y);

            if (pc.isCornerDetectionEnabled != 0U) {
                coords.y = uv.y;
                detectHorizontalCornerPattern(weights.rg, coords.xyzy, d);
            }
        } else {
            e.r = 0.0; // Skip vertical processing
        }
    }

    // Edge at the left
    if (e.r > 0.0) {
        vec2 d;

        // Distance to the top and the crossing edges there
        vec3 coords;
        coords.y = searchYUp(offset1.xy, offset2.z);
        coords.x = offset0.x;
        d.x = coords.y;
        float e1 = sampleEdges(coords.xy).g;

        // Distance to the bottom end
        coords.z = searchYDown(offset1.zw, offset2.w);
        d.y = coords.z;

        d = abs(round(gRtMetrics.ww * d - pixelCoord.yy));
        vec2 sqrtD = sqrt(d);

        float e2 = textureLodOffset(gEdges, coords.xz, 0.0, ivec2(0, 1)).g;

        weights.ba = area(sqrtD, e1, e2, kSubsampleIndices.x);

        if (pc.isCornerDetectionEnabled != 0U) {
            coords.x = uv.x;
            detectVerticalCornerPattern(weights.ba, coords.xyxz, d);
        }
    }

    outWeights = weights;
}
