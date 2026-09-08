struct PixelInput
{
    float3 color : COLOR;
};

float4 main(PixelInput pixelInput) : SV_TARGET
{
    float3 inColor = pixelInput.color;

    return float4(inColor, 1.0f);
}