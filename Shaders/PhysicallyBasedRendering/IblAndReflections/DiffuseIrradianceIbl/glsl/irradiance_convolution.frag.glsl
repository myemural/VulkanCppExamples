#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 directionVector;

layout(set = 0, binding = 0) uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{
    // Cubemap direction = surface normal
    vec3 N = normalize(directionVector);

    // Orthonormal basis construction
    vec3 helperUp = abs(N.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(helperUp, N));
    vec3 bitangent = cross(N, tangent);

    // Lower = More quality
    // Higher = Faster
    const float sampleDelta = 0.025;

    // Hemisphere integration
    vec3 irradiance = vec3(0.0);
    float totalWeight = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // Tangent-space hemisphere sample
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);
            vec3 tangentSample = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

            // Convert tangent-space to world-space
            vec3 sampleVec = tangentSample.x * tangent + tangentSample.y * bitangent + tangentSample.z * N;
            sampleVec = normalize(sampleVec);

            // Lambert cosine weighting
            float weight = cosTheta * sinTheta;
            irradiance += texture(environmentMap, sampleVec).rgb * weight;

            totalWeight += weight;
        }
    }

    // Final normalization
    irradiance = PI * irradiance / max(totalWeight, 0.0001);
    outColor = vec4(irradiance, 1.0);
}