cbuffer MatrixBuffer
{
    matrix MatrixWorld;
    matrix MatrixView;
    matrix MatrixProj;
};

struct VertexIn
{
    float4 pos: POSITION;
    float2 uv: TEXCOORD0;
    float3 normal: NORMAL;
};

struct VertexOut
{
    float4 pos: SV_POSITION;
    float2 uv: TEXCOORD0;
    float3 normal: NORMAL;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    vin.pos.w = 1;
    vout.pos = mul(vin.pos, MatrixWorld);
    vout.pos = mul(vout.pos, MatrixView);
    vout.pos = mul(vout.pos, MatrixProj);
    vout.uv = vin.uv;
    vout.normal = normalize(mul(vin.normal, (float3x3)MatrixWorld));
    return vout;
}