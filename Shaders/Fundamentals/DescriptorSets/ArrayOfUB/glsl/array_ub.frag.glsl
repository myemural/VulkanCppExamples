#version 450

// ------------------------------------------------------------------------
// Author: Mustafa Yemural
// Description:
// ------------------------------------------------------------------------
// Copyright (c) 2025 Mustafa Yemural - www.mustafayemural.com
// Licensed under the MIT License.
// ------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

layout(location = 0) in flat int outIndex;

void main()
{
    if (outIndex == 0) {
        outColor = vec4(1.0, 0.0, 0.0, 1.0);
    } else if (outIndex == 1) {
        outColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else if (outIndex == 2) {
        outColor = vec4(0.0, 0.0, 1.0, 1.0);
    } else {
        outColor = vec4(1.0, 0.0, 1.0, 1.0);
    }

}
