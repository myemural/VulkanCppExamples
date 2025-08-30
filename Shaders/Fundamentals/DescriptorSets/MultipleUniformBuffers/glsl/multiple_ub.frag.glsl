#version 450

layout(location = 0) out vec4 outColor;

layout(set=0, binding=0) uniform ScreenUbo { vec2 screenSize; } screenUbo;
layout(set=0, binding=1) uniform UBO0 { vec3 color; } ubo0;
layout(set=0, binding=2) uniform UBO1 { vec3 color; } ubo1;
layout(set=0, binding=3) uniform UBO2 { vec3 color; } ubo2;
layout(set=0, binding=4) uniform UBO3 { vec3 color; } ubo3;

void main()
{
    vec2 uv = gl_FragCoord.xy / screenUbo.screenSize;
    bool left  = uv.x < 0.5;
    bool lower = uv.y < 0.5;

    if ( left &&  lower ) outColor = vec4(ubo0.color, 1.0);
    if (!left &&  lower ) outColor = vec4(ubo1.color, 1.0);
    if ( left && !lower ) outColor = vec4(ubo2.color, 1.0);
    if (!left && !lower ) outColor = vec4(ubo3.color, 1.0);
}