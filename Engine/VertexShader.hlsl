cbuffer Transforms : register(b0)
{
    float4x4 mvp;
};

struct VertexInput
{
    float3 inPos : POSITION;
    float3 inColor : COLOR;
};

struct VertexOutput
{
    float3 color : COLOR;
    float4 position : SV_POSITION;
};

VertexOutput main(VertexInput vertexInput)
{
    float3 inColor = vertexInput.inColor;
    float3 inPos = vertexInput.inPos;
    
    VertexOutput output;
    output.position = mul(float4(inPos, 1.0f), mvp);
    output.color = inColor;
    return output;
}