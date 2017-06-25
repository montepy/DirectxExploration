float4 VS(float4 inPos: POSITION) : SV_POSITION{
	return inPos;
}

float4 PS(float4 color: COLOR) : SV_TARGET{
	return color;
}