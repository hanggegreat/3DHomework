#include "LightHelper.hlsli"

Texture2D g_Tex : register(t0);
TextureCube g_TexCube : register(t1);
SamplerState g_SamLinear : register(s0);


cbuffer CBChangesEveryDrawing : register(b0)
{
	Material g_Material;
	matrix g_World;
	matrix g_WorldInvTranspose;
	matrix g_WorldViewProj;
	int g_IsShadow;
}

cbuffer CBChangesEveryFrame : register(b1)
{
	matrix g_View;
	float3 g_EyePosW;
}

cbuffer CBChangesOnResize : register(b2)
{
	matrix g_Proj;
}

cbuffer CBChangesRarely : register(b3)
{
	matrix g_Shadow;
	DirectionalLight g_DirLight;
	PointLight g_PointLight;
	SpotLight g_SpotLight;
}



struct VertexPosNormalTex
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
};

struct VertexPosHWNormalTex
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION; // 在世界中的位置
    float3 NormalW : NORMAL; // 法向量在世界中的方向
    float2 Tex : TEXCOORD;
};

struct VertexPos
{
	float3 PosL : POSITION;
};

struct VertexPosHL
{
	float4 PosH : SV_POSITION;
	float3 PosL : POSITION;
};









