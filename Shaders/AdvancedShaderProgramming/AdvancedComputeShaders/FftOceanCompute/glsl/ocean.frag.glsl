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
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in float fragFoam;

struct MeshMaterialData
{
    vec4 diffuseColor;
    vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
    float uvScale;
    int diffuseMap;
    int normalMap;
};

layout(std430, binding = 1) readonly buffer MeshMaterialDataBuffer {
    MeshMaterialData meshMaterials[];
};

layout(std140, set = 0, binding = 2) uniform LightUBO
{
    vec4 lightDirection; // xyz = Light Direction
    vec4 lightColor;     // rgb = Light Color
    vec4 sceneParams;    // x = Time, y = Foam Intensity, z = Fog Start, w = Fog End
} light;

layout(set = 0, binding = 3) uniform sampler2D uCombinedSamplers[];

layout(set = 0, binding = 4) uniform samplerCube uSkybox;

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

// Water body colors constants
const vec3 kTroughColor = vec3(0.004, 0.03, 0.04);
const vec3 kCrestColor = vec3(0.03, 0.175, 0.165);
const vec3 kScatterColor = vec3(0.06, 0.32, 0.23);

// Reflection control constants
const float kReflectance = 0.02; // Water F0
const float kMaxReflection = 0.68; // Fresnel ceiling
const float kReflectionStrength = 0.8;
const float kReflectionDesaturate = 0.35;

// Shading constants
const vec3 kFoamColor = vec3(0.88, 0.93, 0.95);
const float kRippleStrength = 0.3;
const float kShadedSide = 0.45; // How dark the faces turned away from the sun get
const float kFogMax = 0.75; // Never fade completely into the sky
const float kExposure = 1.6;

vec3 sampleSky(vec3 direction)
{
    return texture(uSkybox, vec3(direction.x, direction.y, -direction.z)).rgb;
}

void main()
{
    const MeshMaterialData meshMatInfo = meshMaterials[pc.objectId];
    const float time = light.sceneParams.x;

    const vec3 toCamera = pc.cameraPosition.xyz - fragPos;
    const float distanceToCamera = length(toCamera);
    vec3 viewDir = toCamera / max(distanceToCamera, 1.0e-4);
    vec3 lightDir = normalize(-light.lightDirection.xyz);

    vec3 normalWorldSpace = normalize(fragNormal);

    // Two scrolling samples of the detail normal map add ripples below FFT grid resolution
    const float detailFade = 1.0 - smoothstep(40.0, 160.0, distanceToCamera);
    if (meshMatInfo.normalMap != -1 && detailFade > 0.001) {
        const vec2 uvA = fragUv * meshMatInfo.uvScale + vec2(0.03, 0.017) * time;
        const vec2 uvB = fragUv * meshMatInfo.uvScale * 2.13 - vec2(0.021, 0.035) * time;

        const vec3 rippleA = texture(uCombinedSamplers[nonuniformEXT(meshMatInfo.normalMap)], uvA).rgb * 2.0 - 1.0;
        const vec3 rippleB = texture(uCombinedSamplers[nonuniformEXT(meshMatInfo.normalMap)], uvB).rgb * 2.0 - 1.0;
        const vec3 ripple =normalize(rippleA + rippleB);

        // The ocean plane is axis-aligned, so a simple tangent basis is enough
        const vec3 tangent = normalize(vec3(1.0, 0.0, 0.0) - normalWorldSpace * normalWorldSpace.x);
        vec3 bitangent = cross(normalWorldSpace, tangent);
        const mat3 tbn = mat3(tangent, bitangent, normalWorldSpace);

        const float strength = kRippleStrength * detailFade;
        normalWorldSpace = normalize(tbn * normalize(vec3(ripple.xy * strength, ripple.z)));
    }

    // Environment reflection, clamped to stay above horizon
    vec3 reflectDir = reflect(-viewDir, normalWorldSpace);
    reflectDir.y = max(reflectDir.y, 0.02);
    vec3 reflectedColor = sampleSky(normalize(reflectDir));

    // Desaturating and diming the reflection separates water from the sky
    const float reflectedLuma = dot(reflectedColor, vec3(0.2126, 0.7152, 0.722));
    reflectedColor = mix(reflectedColor, vec3(reflectedLuma), kReflectionDesaturate) * kReflectionStrength;

    // Schlick approximation
    const float fresnel = min(kReflectance + (1.0 - kReflectance) *
                          pow(1.0 - max(dot(normalWorldSpace, viewDir), 0.0), 5.0), kMaxReflection);

    // Water body color: Deeper troughs are darker, crests scatter light towards the viewer
    const float crest = clamp(fragPos.y * 0.3 + 0.5, 0.0, 1.0);
    const float facing = max(dot(normalWorldSpace, lightDir), 0.0);
    const float sideShade = mix(kShadedSide, 1.0, facing);
    const float scatter = crest * crest * pow(max(dot(viewDir, -lightDir), 0.0), 2.0);

    vec3 waterColor = mix(kTroughColor, kCrestColor, crest) * sideShade;
    waterColor += kScatterColor * scatter * 1.2;
    waterColor += light.lightColor.rgb * facing * 0.04;
    waterColor += meshMatInfo.ambientStrength * sampleSky(vec3(0.0, 1.0, 0.0));

    // Sun glitter
    const vec3 halfDir = normalize(lightDir + viewDir);
    const float specular = pow(max(dot(normalWorldSpace, halfDir), 0.0), meshMatInfo.shininess);

    vec3 color = mix(waterColor, reflectedColor, fresnel);
    color += light.lightColor.rgb * specular * meshMatInfo.specularStrength;

    // Foam on breaking crests
    const float foam  = clamp(fragFoam, 0.0, 1.0) * light.sceneParams.y;
    color = mix(color, kFoamColor * (0.4 + 0.6 * facing), foam);

    // Horizon fade, hides the finite patch edge and mimics aerial perspective
    const float fog = smoothstep(light.sceneParams.z, light.sceneParams.w, distanceToCamera) * kFogMax;
    const vec3 horizonColor = sampleSky(normalize(vec3(-viewDir.x, 0.03, -viewDir.z)));
    color = mix(color, horizonColor, fog);

    // Exposure tone mapping
    color = vec3(1.0) - exp(-color * kExposure);

    outColor = vec4(color, 1.0);
}
