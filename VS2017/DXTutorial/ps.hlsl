struct PixelIn
{
    float4 pos: SV_POSITION;
    float4 color: COLOR;
};

float4 PS(PixelIn pin) : SV_TARGET
{
	return pin.color;
}