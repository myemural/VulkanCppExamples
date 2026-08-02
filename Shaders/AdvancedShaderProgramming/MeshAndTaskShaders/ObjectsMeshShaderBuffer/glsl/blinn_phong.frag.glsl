#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------


layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragWorldNormal;

// Scene-wide rendering parameters that contains common camera, light and material informations
layout(std140, set = 0, binding = 3) uniform SceneUBO {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
    vec4 lightDirection;
    vec4 lightColor;
    vec4 diffuseColor;
    vec4 specularColor;
    float ambientStrength;
    float shininess;
    float specularStrength;
} scene;

void main()
{
    vec3 normal = normalize(fragWorldNormal);
    vec3 lightDir = normalize(-scene.lightDirection.xyz);
    vec3 viewDir = normalize(scene.cameraPosition.xyz - fragWorldPos);

    // Ambient
    vec3 ambient = scene.ambientStrength * scene.diffuseColor.rgb;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * scene.lightColor.rgb * scene.diffuseColor.rgb;

    // Specular
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), scene.shininess);
    vec3 specular = scene.specularStrength * spec * scene.lightColor.rgb * scene.specularColor.rgb;

    outColor = vec4(ambient + diffuse + specular, 1.0);
}
