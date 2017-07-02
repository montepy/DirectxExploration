
cbuffer cbPerObject {
	float4x4 WVP;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 Color : COLOR;
};

VS_OUTPUT VS(float4 inPos: POSITION,float4 inColor: COLOR) {
	VS_OUTPUT output;
output.Color = inColor;
output.Pos = mul(inPos, WVP);
	return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET{
	return input.Color;
}
