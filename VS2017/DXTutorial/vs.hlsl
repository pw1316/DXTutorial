cbuffer MatrixBuffer
{
    matrix MatrixWorld;
    matrix MatrixView;
    matrix MatrixProj;
};

struct VertexIn
{
    float4 pos: POSITION;
    float4 color: COLOR;
};

struct VertexOut
{
    float4 pos: SV_POSITION;
    float4 color: COLOR;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    vin.pos.w = 1;
    vout.pos = mul(vin.pos, MatrixWorld);
    vout.pos = mul(vout.pos, MatrixView);
    vout.pos = mul(vout.pos, MatrixProj);
    vout.color = vin.color;
    return vout;
}