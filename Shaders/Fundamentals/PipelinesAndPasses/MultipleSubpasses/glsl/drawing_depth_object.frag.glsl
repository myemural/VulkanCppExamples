#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput depthInput;

void main()
{
    float zNear = 0.1;
    float zFar  = 20.0;
    float depth = subpassLoad(depthInput).r;

    // Transform to view-space Z
    float linearDepth = (zFar * zNear) / (zFar - depth * (zFar - zNear));

    // Normalize linearDepth to 0-1
    linearDepth = clamp((linearDepth - zNear) / (zFar - zNear), 0.0, 1.0);

    // Add power to increase contrast
    vec3 color = vec3(pow(linearDepth, 3.0));
    outColor = vec4(color, 1.0);
}