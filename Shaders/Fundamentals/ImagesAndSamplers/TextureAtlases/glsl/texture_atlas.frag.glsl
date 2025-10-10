#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragUV;

layout(set = 0, binding = 0) uniform sampler2D uAtlas;

void main()
{
    outColor = texture(uAtlas, fragUV);
}