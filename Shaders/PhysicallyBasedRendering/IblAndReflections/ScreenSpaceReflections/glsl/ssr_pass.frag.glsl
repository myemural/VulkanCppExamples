#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D gPosition;
layout(set = 0, binding = 1) uniform sampler2D gNormal;
layout(set = 0, binding = 2) uniform sampler2D gMetallicRoughness;
layout(set = 0, binding = 3) uniform sampler2D sceneColor;

layout(push_constant) uniform SsrPushConstants {
    mat4 proj;
    float maxDistance;     // Maximum ray distance in view-space
    float thickness;       // Depth tolerance (reduces false positives)
    int maxSteps;          // Ray march max step count
    int binarySearchSteps; // Step count for intersection refinement
    uint isSsrEnabled;     // 0: SSR Disabled, 1: SSR Enabled
} pc;

// Fresnel calculation function
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Screen edge fading, for reducing edge imperfections
float screenEdgeFade(vec2 uv)
{
    const float fadeWidth = 0.08;
    vec2 f = smoothstep(vec2(0.0), vec2(fadeWidth), uv) * smoothstep(vec2(1.0), vec2(1.0 - fadeWidth), uv);
    return f.x * f.y;
}

// Binary search
vec3 binarySearchSSR(vec3 startPos, vec3 endPos)
{
    vec3 lo = startPos;
    vec3 hi = endPos;

    for (int i = 0; i < pc.binarySearchSteps; ++i)
    {
        vec3 mid = (lo + hi) * 0.5;

        vec4 clip = pc.proj * vec4(mid, 1.0);
        vec2 uv = (clip.xy / clip.w) * 0.5 + 0.5;

        vec3 surfacePos = texture(gPosition, uv).xyz;

        if (mid.z < surfacePos.z) {
            hi = mid; // Ray is behind the surface
        } else {
            lo = mid; // Ray is in front of the surface
        }
    }

    vec3 finalPos  = (lo + hi) * 0.5;
    return finalPos;
}

// Main ray marching function for SSR calculation
// Reference: https://imanolfotia.com/blog/1
vec3 raymarchSSR(vec3 origin, vec3 dir, out bool hit)
{
    hit = false;

    vec3 stepVec = dir * (pc.maxDistance / float(pc.maxSteps));
    vec3 prevPos = origin;
    vec3 curPos  = origin + stepVec * 0.5;

    for (int i = 0; i < pc.maxSteps; ++i)
    {
        curPos += stepVec;

        // Transform current position from view-space to clip-space
        vec4 clip = pc.proj * vec4(curPos, 1.0);

        // If the position is behind the camera, stop ray marching
        if (clip.w <= 0.0) {
            break;
        }

        vec2 uv = (clip.xy / clip.w) * 0.5 + 0.5;

        // If the position is out of screen, stop ray marching
        if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) {
            break;
        }

        // Read the view-space position of the stage surface in this UV
        vec3 surfacePosView = texture(gPosition, uv).xyz;

        // Background pixel (no geometry): gPosition cleared to 0, since all valid geometry is in front of the camera
        if (surfacePosView.z >= -0.001) {
            continue;
        }

        // Intersection test (in view-space):
        if (curPos.z < surfacePosView.z && curPos.z > surfacePosView.z - pc.thickness)
        {
            vec3 finalPos = binarySearchSSR(prevPos, curPos);
            vec4 finalClip = pc.proj * vec4(finalPos, 1.0);
            vec2 finalUV = (finalClip.xy / finalClip.w) * 0.5 + 0.5;

            if (any(lessThan(finalUV, vec2(0.0))) || any(greaterThan(finalUV, vec2(1.0))))
            {
                break;
            }

            // Reflection distance fade, distant reflections are weaker
            const float distFade = 1.0 - clamp(length(finalPos - origin) / pc.maxDistance, 0.0, 1.0);

            hit = true;
            return texture(sceneColor, finalUV).rgb * screenEdgeFade(finalUV) * distFade;
        }

        prevPos = curPos;
    }

    return vec3(0.0);
}

void main()
{
    // Sampling G-Buffer and scene color
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(gPosition, 0));

    vec3 posView = texture(gPosition, uv).xyz;
    vec3 normView = normalize(texture(gNormal, uv).rgb);
    float metallic = texture(gMetallicRoughness, uv).r;
    float roughness = texture(gMetallicRoughness, uv).g;
    vec4 scene = texture(sceneColor, uv);

    // Calculate view direction and reflect direction
    vec3 viewDir = normalize(-posView);
    vec3 reflDir = reflect(-viewDir, normView);

    // If we don't need reflection, diretly skip the ray marching, conditions:
    // - z >= 0: There is no geometry in front of the camera
    // - roughness > 0.7: Too rough surface
    // - metallic < 0.01: Too dielectric surface
    // - refDir.z > 0.0: Reflection towards the camera
    // - isSsrEnabled == 0: SSR disabled
    if (posView.z >= -0.001 || roughness > 0.7 || metallic < 0.01 || reflDir.z > 0.0 || pc.isSsrEnabled == 0)
    {
        outColor = scene;
        return;
    }

    // Fresnel calculation
    vec3 F0 = mix(vec3(0.04), vec3(1.0), metallic);
    float fresnel = fresnelSchlick(max(dot(normView, viewDir), 0.0), F0).r;

    // Ray marching
    bool hit;
    vec3 reflColor = raymarchSSR(posView, reflDir, hit);

    if (hit)
    {
        float roughnessFade = pow(1.0 - roughness, 2.0);
        float weight = clamp(fresnel * roughnessFade, 0.0, 1.0);
        outColor = vec4(mix(scene.rgb, reflColor, weight), scene.a);
        return;
    }

    // Fallback
    outColor = scene;
}