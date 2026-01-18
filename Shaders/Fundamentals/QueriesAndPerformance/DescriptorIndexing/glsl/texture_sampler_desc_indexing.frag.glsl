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

layout(location = 0) in vec2 fragUV;
layout(location = 1) flat in uint textureIndex;

layout(set = 0, binding = 1) uniform sampler2D uCombinedSamplers[];

void main()
{
    outColor = texture(uCombinedSamplers[nonuniformEXT(textureIndex)], fragUV);
}