struct UBO
{
    float3 color;
};

cbuffer colorUb : register(b0, space0) { UBO colorUb; }

float4 main() : SV_Target
{
    return float4(colorUb.color, 1.0);
}