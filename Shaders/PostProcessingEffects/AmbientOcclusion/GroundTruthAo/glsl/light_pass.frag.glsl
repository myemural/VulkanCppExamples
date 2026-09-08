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
layout(set = 0, binding = 1) uniform sampler2D gAlbedo;
layout(set = 0, binding = 2) uniform sampler2D gNormal;
layout(set = 0, binding = 3) uniform sampler2D uOcclusionRaw;
layout(set = 0, binding = 4) uniform sampler2D uOcclusionBlurred;

layout(set = 0, binding = 5) uniform DirectionalLightBuffer
{
    vec4 lightDirection; // xyz = Light Direction (View-Space)
    vec4 lightColor;     // xyz = Light Color
} light;

layout(push_constant) uniform LightingPushConstants {
    // Debug mode for output
    // 0: Off
    // 1: Albedo Output
    // 2: Normalized View-Space Normal Output
    // 3: Raw Occlusion Factor Output
    // 4: Blurred Occlusion Factor Output
    // 5: Only Ambient Term Output
    uint debugMode;
    uint ssaoEnabled;
    float ambientStrength;
} pc;

void main()
{
    // Constants
    const float specularStrength = 0.3;
    const float shininess = 64.0;

    // Sampling G-Buffer
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(gPosition, 0));

    vec4 positionSample = texture(gPosition, uv);
    vec3 albedo = texture(gAlbedo, uv).rgb;
    float occlusionRaw = texture(uOcclusionRaw, uv).r;
    float occlusionBlurred = texture(uOcclusionBlurred, uv).r;
    float ao = pc.ssaoEnabled == 1U ? occlusionBlurred : 1.0;

    if (pc.debugMode == 3U) {
        outColor = vec4(vec3(occlusionRaw), 1.0);
        return;
    }

    if (pc.debugMode == 4U) {
        outColor = vec4(vec3(occlusionBlurred), 1.0);
        return;
    }

    // Background
    if (positionSample.w < 0.5) {
        outColor = vec4(albedo, 1.0);
        return;
    }


    vec3 fragPosView = positionSample.xyz;
    vec3 normalView = normalize(texture(gNormal, uv).xyz);

    // Lighting vectors in view space
    vec3 lightDirView = normalize(-light.lightDirection.xyz);
    vec3 viewDir = normalize(-fragPosView);
    vec3 halfDir = normalize(lightDirView + viewDir);

    // Ambient calculation (the only term that the occlusion factor is applied to)
    vec3 ambient = pc.ambientStrength * albedo * ao;

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalView, lightDirView), 0.0);
    vec3 diffuse = diff * light.lightColor.rgb * albedo;

    // Specular calculation
    float spec = pow(max(dot(normalView, halfDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * light.lightColor.rgb;

    if (pc.debugMode == 1) {
        outColor = vec4(albedo, 1.0);
    } else if (pc.debugMode == 2) {
        outColor = vec4(normalView * 0.5 + 0.5, 1.0);
    } else if (pc.debugMode == 5) {
        outColor = vec4(ambient, 1.0);
    } else {
        outColor = vec4(ambient + diffuse + specular, 1.0);
    }
}
