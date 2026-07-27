#version 450
#extension GL_EXT_nonuniform_qualifier : require

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in mat3 fragTBN;
layout(location = 5) in float inWaveHeight; // Signed wave displacement

struct MeshMaterialData
{
    vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
    float uvScale;
    int normalMap;
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(std140, set = 0, binding = 2) uniform LightUBO
{
    vec4 lightDirection; // xyz = Light Direction
    vec4 lightColor;     // rgb = Light Color
} light;

layout(set = 0, binding = 3) uniform sampler2D uCombinedSamplers[];

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
    float tessellationLevel;
    float displacementLevel;
    float time;
} pc;

// Deep/shallow water tint colors
const vec3 kDeepWaterColor = vec3(0.02, 0.10, 0.18);
const vec3 kShallowWaterColor = vec3(0.06, 0.32, 0.38);

void main()
{
    // Get mesh info
    const MeshMaterialData meshMatInfo = meshMaterials[pc.objectId];

    // Calculate scaled UV
    const vec2 scaledUv = fragUv * meshMatInfo.uvScale;

    // Normal map: Two copies of the ripple normal map scrolled in opposite directions and averaged, so the ripples
    // read as steady motion instead of a texture sliding rigidly across the surface
    vec3 normalWorldSpace;
    if (meshMatInfo.normalMap != -1) {
        vec2 flowA = scaledUv + vec2(0.05, 0.03) * pc.time;
        vec2 flowB = scaledUv * 1.3 - vec2(0.04, 0.06) * pc.time;

        vec3 nA = texture(uCombinedSamplers[nonuniformEXT(meshMatInfo.normalMap)], flowA).rgb * 2.0 - 1.0;
        vec3 nB = texture(uCombinedSamplers[nonuniformEXT(meshMatInfo.normalMap)], flowB).rgb * 2.0 - 1.0;
        vec3 normalTangent = normalize(vec3(nA.xy + nB.xy, nA.z * nB.z));

        normalWorldSpace = normalize(fragTBN * normalTangent);
    }
    else
    {
        // Fallback: Geometric normal
        normalWorldSpace = normalize(fragTBN[2]);
    }

    // Base color: Slope-based deep/shallow water tint
    float slope = 1.0 - clamp(dot(normalize(fragTBN[2]), vec3(0.0, 1.0, 0.0)), 0.0, 1.0);
    vec3 diffuseColor = mix(kDeepWaterColor, kShallowWaterColor, slope);

    // Normalizing light direction
    vec3 normalizedLightDir = normalize(-light.lightDirection.xyz);

    // Normalizing view direction (camera position)
    vec3 normalizedView = normalize(pc.cameraPosition.xyz - fragPos);

    // Ambient
    vec3 ambient = meshMatInfo.ambientStrength * diffuseColor;

    // Diffuse
    float diff = max(dot(normalWorldSpace, normalizedLightDir), 0.0);
    vec3 diffuse = diff * light.lightColor.rgb * diffuseColor;

    // Specular
    vec3 halfDir = normalize(normalizedLightDir + normalizedView);
    float spec = pow(max(dot(normalWorldSpace, halfDir), 0.0), meshMatInfo.shininess);
    vec3 specular = meshMatInfo.specularStrength * spec * light.lightColor.rgb * meshMatInfo.specularColor.rgb;

    vec3 finalColor = ambient + diffuse + specular;

    // Foam: Whiten the surface near wave crests
    float foamMask = smoothstep(0.55, 1.5f, inWaveHeight);
    finalColor = mix(finalColor, vec3(1.0), foamMask);

    outColor = vec4(finalColor, 1.0);
}
