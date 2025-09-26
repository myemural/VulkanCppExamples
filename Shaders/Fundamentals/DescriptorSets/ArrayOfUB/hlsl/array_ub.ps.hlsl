struct PSInput
{
    float4 Position : SV_POSITION;
    nointerpolation int ModelIndex : TEXCOORD0;
};

float4 main(PSInput input) : SV_Target
{
    if (input.ModelIndex == 0) {
        return float4(1.0, 0.0, 0.0, 1.0);
    } else if (input.ModelIndex == 1) {
        return float4(0.0, 1.0, 0.0, 1.0);
    } else if (input.ModelIndex == 2) {
        return float4(0.0, 0.0, 1.0, 1.0);
    } else if (input.ModelIndex == 3) {
        return float4(1.0, 0.0, 1.0, 1.0);
    }

    return float4(0.0, 0.0, 0.0, 1.0);
}