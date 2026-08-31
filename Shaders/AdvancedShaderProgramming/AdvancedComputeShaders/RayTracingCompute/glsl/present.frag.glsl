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

layout(location = 0) in vec2 fragUv;

layout(set = 0, binding = 0) uniform sampler2D radianceImage;

void main()
{
    outColor = vec4(texture(radianceImage, fragUv).rgb, 1.0);
}
