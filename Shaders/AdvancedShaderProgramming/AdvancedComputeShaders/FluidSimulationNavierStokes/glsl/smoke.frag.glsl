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

layout(location = 0) in vec3  fragPos;

layout(std140, set = 0, binding = 2) uniform SceneUBO
{
    vec4 lightDirection; // xyz = Light Direction
    vec4 lightColor;     // xyz = Light Color
    vec4 volumeMin;      // xyz = Smoke Volume Min Corner, w = Smoke Absorption
    vec4 volumeMax;      // xyz = Smoke Volume Max Corner
} scene;

layout(set = 0, binding = 3) uniform sampler3D smokeVolume; // w channel holds the density

layout(push_constant) uniform MeshPushConstants {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    uint objectId;
} pc;

const int kViewSteps = 96;
const int kShadowSteps = 8;

const vec3 kSmokeAlbedo = vec3(0.84, 0.82, 0.8);
const vec3 kAmbientColor = vec3(0.24, 0.27, 0.33);

// Slab test against the volume box, returns the entry and exist distances
vec2 intersectVolume(vec3 origin, vec3 direction)
{
    vec3 inverseDir = 1.0 / (direction + 1e-8);
    vec3 t0 = (scene.volumeMin.xyz - origin) * inverseDir;
    vec3 t1 = (scene.volumeMax.xyz - origin) * inverseDir;
    vec3 tSmall = min(t0, t1);
    vec3 tLarge = max(t0, t1);

    return vec2(max(max(tSmall.x, tSmall.y), tSmall.z), min(min(tLarge.x, tLarge.y), tLarge.z));
}

float sampleDensity(vec3 worldPos)
{
    vec3 uvw = (worldPos - scene.volumeMin.xyz) / (scene.volumeMax.xyz - scene.volumeMin.xyz);
    return textureLod(smokeVolume, uvw, 0.0).w;
}

float lightTransmittence(vec3 worldPos, vec3 toLight)
{
    vec2 hit = intersectVolume(worldPos, toLight);
    float start = max(hit.x, 0.0);

    if (hit.y <= start) {
        return 1.0;
    }

    float stepSize = (hit.y - start) / float(kShadowSteps);
    float opticalDepth = 0.0;

    for (int i = 0; i < kShadowSteps; ++i) {
        vec3 samplePos = worldPos + toLight * (start + (float(i) + 0.5) * stepSize);
        opticalDepth += sampleDensity(samplePos);
    }

    return exp(-opticalDepth * stepSize * scene.volumeMin.w);
}

void main()
{
    // The pipeline draws the inner faces of the box, so fragPos is always behind the camera ray
    vec3 rayOrigin = pc.cameraPosition.xyz;
    vec3 rayDirection = normalize(fragPos - rayOrigin);

    vec2 hit = intersectVolume(rayOrigin, rayDirection);
    float marchStart = max(hit.x, 0.0);

    if (hit.y <= marchStart) {
        discard;
    }

    vec3 toLight = normalize(-scene.lightDirection.xyz);
    float stepSize = (hit.y - marchStart) / float(kViewSteps);

    float transmittence = 1.0;
    vec3 scattered = vec3(0.0);

    for (int i = 0; i < kViewSteps; ++i) {
        vec3 samplePos = rayOrigin + rayDirection * (marchStart + (float(i) + 0.5) * stepSize);
        float density = sampleDensity(samplePos);

        // Skipping empty space keeps the expensive shadow march off most of the samples
        if (density > 0.02) {
            float alpha = 1.0 - exp(-density * stepSize * scene.volumeMin.w);
            float shadow = lightTransmittence(samplePos, toLight);
            vec3 lit = kSmokeAlbedo * (kAmbientColor + scene.lightColor.rgb * shadow);

            scattered += transmittence * alpha * lit;
            transmittence *= 1.0 - alpha;

            if (transmittence < 0.01) {
                break;
            }
        }
    }

    outColor = vec4(scattered, 1.0 - transmittence);
}
