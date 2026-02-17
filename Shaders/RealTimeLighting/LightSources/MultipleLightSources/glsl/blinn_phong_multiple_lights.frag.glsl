#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

// Light types
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT       1
#define LIGHT_TYPE_SPOT        2

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;

struct MeshMaterialData
{
    vec4 diffuseColor;
    vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(constant_id = 0) const uint LIGHT_COUNT = 0;

struct LightData
{
    vec4 lightPosition;    // xyz = Light Position (unused for directional lights)
    vec4 lightDirection;   // xyz = Light Direction (normalized, unused for point lights)
    vec4 lightColor;       // rgb = Light Color, a = Light Intensity

    vec4 lightTypeParams;  // x = Light Type (0: Directional, 1: Point: 2: Spot)
    vec4 pointLightParams; // x = Constant Factor, y = Linear Factor, z = Quadratic Factor
    vec4 spotlightParams;  // x = cos(innerCutoffAngle), y = cos(outerCutoffAngle)
};

layout(std430, set = 0, binding = 2) readonly buffer LightBuffer
{
    LightData lights[];
};

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

vec3 calculateLight(LightData light, vec3 normalizedNormal, vec3 fragmentPosition, vec3 normalizedView)
{
    // Get mesh info
    const MeshMaterialData meshMatInfo = meshMaterials[pc.objectId];

    // Get light type
    int type = int(light.lightTypeParams.x);

    // Calculate normalized light direction
    vec3 normalizedLightDir;
    if (type == LIGHT_TYPE_DIRECTIONAL)
    {
        normalizedLightDir = normalize(-light.lightDirection.xyz);
    }
    else
    {
        normalizedLightDir = normalize(light.lightPosition.xyz - fragmentPosition);
    }

    float attenuation = 1.0;

    // Calculate attenuation contribuiton for spotlight
    if (type == LIGHT_TYPE_POINT)
    {
        float dist = length(light.lightPosition.xyz - fragmentPosition);
        attenuation = 1.0 / (light.pointLightParams.x + light.pointLightParams.y * dist + light.pointLightParams.z * dist * dist);
    }

    // Calculate spotlight calculation (with no attenuation)
    if (type == LIGHT_TYPE_SPOT)
    {
        vec3 spotDir = normalize(-light.lightDirection.xyz);
        float theta = dot(normalizedLightDir, spotDir);
        float epsilon = light.spotlightParams.x - light.spotlightParams.y;

        float spotFactor = clamp((theta - light.spotlightParams.y) / epsilon, 0.0, 1.0);

        attenuation *= spotFactor;
    }

    // Diffuse (Lambert) calculation
    float diff = max(dot(normalizedNormal, normalizedLightDir), 0.0);
    vec3 diffuse = diff * light.lightColor.rgb * meshMatInfo.diffuseColor.rgb;

    // Specular calculation
    vec3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalizedNormal, halfDir), 0.0), meshMatInfo.shininess);
    vec3 specular = meshMatInfo.specularStrength * spec * light.lightColor.rgb * meshMatInfo.specularColor.rgb;

    // Final color
    vec3 finalColor = attenuation * light.lightColor.a * (diffuse + specular);
    return finalColor;
}

void main()
{
    // Normalizing normal
    vec3 normalizedNormal = normalize(fragNormal);

    // Normalizing view direction (camera position)
    vec3 normalizedView = normalize(pc.cameraPosition.xyz - fragPos);

    vec3 resultColor = vec3(0.0);
    for (uint i = 0; i < LIGHT_COUNT; i++) {
        resultColor += calculateLight(lights[i], normalizedNormal, fragPos, normalizedView);
    }

    // Ambient calculation
    vec3 ambient = meshMaterials[pc.objectId].ambientStrength * meshMaterials[pc.objectId].diffuseColor.rgb;
    resultColor += ambient;

    outColor = vec4(resultColor, 1.0);
}