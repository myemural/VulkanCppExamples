#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 fragUv;

layout(set = 0, binding = 0) uniform sampler2D gPosition;
layout(set = 0, binding = 1) uniform sampler2D gAlbedo;
layout(set = 0, binding = 2) uniform sampler2D gNormal;

layout(constant_id = 0) const uint LIGHT_COUNT = 0;

struct PointLightData
{
    vec4 lightPosition;    // xyz = Light Position (View-Space)
    vec4 lightColor;       // rgb = Light Color
    vec4 pointLightParams; // x = Constant Factor, y = Linear Factor, z = Quadratic Factor
};

layout(std430, set = 0, binding = 3) readonly buffer PointLightBuffer
{
    PointLightData lights[];
};

vec3 calculateLight(PointLightData light, vec3 albedo, vec3 normalView, vec3 fragPosView, vec3 viewDir)
{
    // Constants
    const float specularStrength = 0.4;
    const float shininess = 128.0;

    float dist = length(light.lightPosition.xyz - fragPosView);
    float attenuation = 1.0 / (light.pointLightParams.x + light.pointLightParams.y * dist + light.pointLightParams.z * dist * dist);;

    vec3 lightDirView = normalize(light.lightPosition.xyz - fragPosView);

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalView, lightDirView), 0.0);
    vec3 diffuse = diff * light.lightColor.rgb * albedo;

    // Specular calculation
    vec3 halfDir = normalize(lightDirView + viewDir);
    float spec = pow(max(dot(normalView, halfDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * light.lightColor.rgb;

    // Final color
    vec3 finalColor = attenuation * diffuse + specular;
    return finalColor;
}

void main()
{
    // Constants
    const float ambientStrength = 0.02;

    // Sampling G-Buffer
    vec3 fragPosView = texture(gPosition, fragUv).xyz;
    vec4 albedoSample = texture(gAlbedo, fragUv);
    vec3 albedo = albedoSample.rgb;
    float opacity = albedoSample.a;

    if (opacity <= 0.001) {
        outColor = vec4(albedo, 1.0);
        return;
    }

    vec3 normalView = texture(gNormal, fragUv).rgb;
    normalView = normalize(normalView * 2.0 - 1.0);

    // Lighting vectors in view space
    vec3 viewDir = normalize(-fragPosView);

    // Ambient calculation
    vec3 ambient = ambientStrength * albedo;

    vec3 resultColor = vec3(0.0);
    for (uint i = 0; i < LIGHT_COUNT; i++) {
        resultColor += calculateLight(lights[i], albedo, normalView, fragPosView, viewDir);
    }
    resultColor += ambient;

    outColor = vec4(resultColor, opacity);
}