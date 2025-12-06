// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

[[vk::input_attachment_index(0)]]
[[vk::binding(0, 0)]]
SubpassInput depthInput;

float4 main() : SV_Target
{
    float zNear = 0.1;
    float zFar  = 20.0;
    float depth = depthInput.SubpassLoad().r;

    // Transform to view-space Z
    float linearDepth = (zFar * zNear) / (zFar - depth * (zFar - zNear));

    // Normalize linearDepth to 0-1
    linearDepth = clamp((linearDepth - zNear) / (zFar - zNear), 0.0, 1.0);

    // Add power to increase contrast
    float3 color = pow(linearDepth.xxx, 3.0);
    return float4(color, 1.0);
}