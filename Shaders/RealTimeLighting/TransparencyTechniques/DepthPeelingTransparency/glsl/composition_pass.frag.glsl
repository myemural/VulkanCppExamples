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

layout(set = 0, binding = 0) uniform sampler2D opaqueTex;
layout(set = 0, binding = 1) uniform sampler2D peelLayers[];

layout(push_constant) uniform CompositionPushConstants {
    int layerCount;
} pc;

void main()
{
    vec4 accum = texture(opaqueTex, fragUv);
    for(int i = pc.layerCount - 1; i >= 0; --i)
    {
        vec4 c = texture(peelLayers[nonuniformEXT(i)], fragUv);
        accum.rgb = c.rgb * c.a + accum.rgb * (1.0 - c.a);
        accum.a   = c.a + accum.a * (1.0 - c.a);
    }

    outColor = accum;
}