#version 450

layout(location = 0) in vec2 inPosition;

layout(set = 0, binding = 0) uniform ModelUBO {
    mat4 model;
} ubo[4];

layout(push_constant) uniform PushConstants {
    int modelIndex;
} pc;

layout(location = 0) out flat int outIndex;

void main()
{
    mat4 currentModel = ubo[pc.modelIndex].model;
    vec4 pos = vec4(inPosition, 0.0, 1.0);
    gl_Position = currentModel * pos;
    outIndex = pc.modelIndex;
}
