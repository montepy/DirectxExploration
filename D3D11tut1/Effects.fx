
cbuffer cbPerObject {
	float4x4 WVP;
	float4x4 World;
};

struct Light {
	float3 dir;
	float4 ambient;
	float4 diffuse;
};

cbuffer cbPerFrame {
	Light light;
};

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal: NORMAL;
};
Texture2D ObjTexture;
SamplerState ObjSamplerState;

VS_OUTPUT VS(float4 inPos: POSITION,float2 texCoord:TEXCOORD,float3 normal:NORMAL) {
	VS_OUTPUT output;
output.texCoord = texCoord;
output.Pos = mul(inPos, WVP);
output.normal = mul(normal, World);
	return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET{
	input.normal = normalize(input.normal);

float4 diffuse = ObjTexture.Sample(ObjSamplerState, input.texCoord);
float3 finalColor;
finalColor = diffuse*light.ambient;
finalColor += saturate(dot(light.dir, input.normal)*light.diffuse*diffuse);

return float4(finalColor,diffuse.a) ;
}

float4 D2D_PS(VS_OUTPUT input) : SV_TARGET
{
	float4 diffuse = ObjTexture.Sample(ObjSamplerState, input.texCoord);

	return diffuse;
}
