#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D gSceneColor;

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(gSceneColor, 0));
    vec3 sceneColor = textureLod(gSceneColor, uv, 0.0).rgb;

    vec3 ldrColor = sceneColor / (sceneColor + vec3(1.0));

    outColor = vec4(ldrColor, 1.0);
}
