
cbuffer cbPerObject {
	float4x4 WVP;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};
Texture2D ObjTexture;
SamplerState ObjSamplerState;

VS_OUTPUT VS(float4 inPos: POSITION,float2 texCoord:TEXCOORD) {
	VS_OUTPUT output;
output.texCoord = texCoord;
output.Pos = mul(inPos, WVP);
	return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET{
	return ObjTexture.Sample(ObjSamplerState,input.texCoord);
}
