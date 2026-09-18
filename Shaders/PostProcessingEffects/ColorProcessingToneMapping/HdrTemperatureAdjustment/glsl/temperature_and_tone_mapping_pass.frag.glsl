#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D gLightPassOutput;

layout(push_constant) uniform ToneMappingPushConstants {
    float exposure; // Manual exposure
    float temperature; // Correlated color temperature in Kelvin, range: [2000, 12000], default: 6500
    float tint; // Green/magenta axis, range: [-1, 1], negative is green, positive is magenta, default: 0.0
    // Temperature mode for output
    // 0: Bypass, adaptation disabled
    // 1: White Balance (corrective)
    // 2: Color Temperature (inverse operation)
    uint mode;
} pc;

// Rec.709 luma coefficients
const vec3 kLumaRec709 = vec3(0.2126, 0.7152, 0.0722);

// The white point that the corrective mode adapts to
const float kReferenceTemperature = 6500.0;

// Maximum Duv offset reached at |tint| == 1
const float kTintDuvScale = 0.05;

// Bradford cone response, from XYZ to LMS
const mat3 kBradfordXyzToLms = mat3(
        0.8951, -0.7502, 0.0389,
        0.2664, 1.7135,  0.0685,
        -0.1614, 0.0367, 0.0296);

// Pre-multiplied linear Rec.709 to/from Bradford LMS
const mat3 kRgbToLms = mat3(
        0.42265804, 0.05569079, 0.02137922,
        0.49136561, 0.96155627,  0.08764394,
        0.02736450,  0.02318934, 0.98074348);

const mat3 kLmsToRgb = mat3(
        2.53844790, -0.14600032,  -0.04228832,
        -1.29348235, 1.11662230,   -0.07159011,
        -0.04024329,-0.02232848, 1.02250725);

const mat3 kAgXInsetMatrix = mat3(
        0.856627153315983, 0.137318972929847, 0.11189821299995,
        0.0951212405381588, 0.761241990602591,  0.0767994186031903,
        0.0482516061458583,  0.101439036467562, 0.811302368396859);

const mat3 kAgXOutsetMatrix = mat3(
        1.1271005818144368, -0.1413297634984383,  -0.14132976349843826,
        -0.11060664309660323, 1.157823702216272,   -0.11060664309660294,
        -0.016493938717834573,-0.016493938717834257, 1.2519364065950405);

const float kAgXMinEv = -12.47393;
const float kAgXMaxEv = 4.026069;

// Black body chromacity in CIE 1931 xy, valid between 1667K and 25000K
vec2 planckianLocusXy(float temperature)
{
    float t = clamp(temperature, 1667.0, 25000.0);
    float t2 = t * t;
    float t3 = t2 * t;

    float x;
    if (t <= 4000.0) {
        x = -0.2661239e9 / t3 - 0.2343589e6 / t2 + 0.8776956e3 / t + 0.179910;
    } else {
        x = -3.0258469e9 / t3 + 2.1070379e6 / t2 + 0.2226347e3 / t + 0.240390;
    }

    float x2 = x * x;
    float x3 = x2 * x;

    float y;
    if (t <= 2222.0) {
        y = -1.1063814 * x3 - 1.34811020 * x2 + 2.18555832 * x - 0.20219683;
    } else if (t <= 4000.0) {
        y = -0.9549476 * x3 - 1.37418593 * x2 + 2.09137015 * x - 0.16748867;
    } else {
        y = 3.0817580 * x3 - 5.87338670 * x2 + 3.75112997 * x - 0.37001483;
    }

    return vec2(x, y);
}

// Convert from CIE 1932 xy to CIE 1960 UCS uv
vec2 xyToUv1960(vec2 xy)
{
    float d = -2.0 * xy.x + 12.0 * xy.y + 3.0;
    return vec2(4.0 * xy.x, 6.0 * xy.y) / d;
}

// Convert from CIE 1960 UCS uv to CIE 1932 xy
vec2 uv1960ToXy(vec2 uv)
{
    float d = 2.0 * uv.x - 8.0 * uv.y + 4.0;
    return vec2(3.0 * uv.x, 2.0 * uv.y) / d;
}

// Chromacity of the selected illuminant, a point on the locus at 'temperature', displaced perpendicular to the locus
// by 'tint'
vec2 whitePointXy(float temperature, float tint)
{
    float t = clamp(temperature, 1667.0, 25000.0);
    float dt = t * 0.01;

    vec2 uv = xyToUv1960(planckianLocusXy(t));

    // Locus tangent by central difference, then rotate 90 degrees
    vec2 tangent = xyToUv1960(planckianLocusXy(t + dt)) - xyToUv1960(planckianLocusXy(t - dt));
    vec2 normal = normalize(vec2(-tangent.y, tangent.x));

    normal *= (normal.y < 0.0) ? -1.0 : 1.0;

    return uv1960ToXy(uv + normal * (tint * kTintDuvScale));
}

// Convertion from chromacity to tristimulus, normalized to Y = 1
vec3 xyToXyz(vec2 xy)
{
    float X = xy.x / xy.y;
    float Y = 1.0;
    float Z = (1.0 - xy.x - xy.y) / xy.y;

    return vec3(X, Y, Z);
}

// Per-cone gains of the von Kries style adaptation
vec3 chromaticAdaptationGains(float temperature, float tint, uint mode)
{
    vec3 selected = kBradfordXyzToLms * xyToXyz(whitePointXy(temperature, tint));
    vec3 reference = kBradfordXyzToLms * xyToXyz(planckianLocusXy(kReferenceTemperature));

    // Mode 1: Adapt the selected illuminant to the reference white
    // Mode 2: Adapt the reference white to the selected illuminant
    return (mode == 1U) ? reference / selected : selected / reference;
}


vec3 adaptColor(vec3 color, vec3 gains)
{
    return kLmsToRgb * (gains * (kRgbToLms * color));
}

vec3 applyWhiteBalance(vec3 color, float temperature, float tint, uint mode)
{
    vec3 gains = chromaticAdaptationGains(temperature, tint, mode);

    // A pure adaptation maps the source white to the destination white, which changes the luminance of neutrals
    // Renormalizing keeps the temperature dial from doubling as a brightness dial
    float normalization = dot(adaptColor(vec3(1.0), gains), kLumaRec709);

    return adaptColor(color, gains) / max(normalization, 1e-6);
}

vec3 agxDefaultContrastApprox(vec3 x)
{
    vec3 x2 = x * x;
    vec3 x4 = x2 * x2;
    return 15.5 * x4 * x2 - 40.14 * x4 * x + 31.96 * x4 - 6.868 * x2 * x + 0.4298 * x2 + 0.1191 * x - 0.00232;
}

// AgX tonemapper
// Returns a display-encoded value, so the result still has to be linearized before it is written to sRGB render target
// Reference: https://iolite-engine.com/blog_posts/minimal_agx_implementation
vec3 tonemapAgx(vec3 color)
{
    vec3 val = kAgXInsetMatrix * color;

    // Log2 encode into the AgX working range
    val = clamp(log2(max(val, vec3(1e-10))), kAgXMinEv, kAgXMaxEv);
    val = (val - kAgXMinEv) / (kAgXMaxEv - kAgXMinEv);

    // Apply the default AgX contrast/look approximation
    val = agxDefaultContrastApprox(val);

    return clamp(kAgXOutsetMatrix * val, 0.0, 1.0);
}

// Linearize the display-encoded AgX output
vec3 agxEotf(vec3 color)
{
    return pow(max(color, vec3(0.0)), vec3(2.2));
}

void main()
{
    vec3 hdrColor = texelFetch(gLightPassOutput, ivec2(gl_FragCoord.xy), 0).rgb;

    if (pc.mode != 0U) {
        hdrColor = applyWhiteBalance(hdrColor, pc.temperature, pc.tint, pc.mode);

        // The adaptation can push saturated colors outside the Rec.709 gamut
        // Clipping to zero is the simplest solution, but real pipeline would gamut compress instead
        hdrColor = max(hdrColor, vec3(0.0));
    }

    // Manual exposure control
    vec3 exposedColor = hdrColor * exp2(pc.exposure);

    // Apply AgX tonemapper
    outColor = vec4(agxEotf(tonemapAgx(exposedColor)), 1.0);
}
