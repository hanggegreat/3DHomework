#include "Basic.hlsli"

// 顶点着色器
VertexPosHWNormalTex VS(VertexPosNormalTex vIn)
{
	VertexPosHWNormalTex vOut;

	matrix viewProj = mul(g_View, g_Proj);
	float4 posW = mul(float4(vIn.PosL, 1.0f), g_World);
	// 若当前在绘制阴影，先进行投影操作
	posW = g_IsShadow ? mul(posW, g_Shadow) : posW;

	vOut.PosH = mul(posW, viewProj);
	vOut.PosW = posW.xyz;
	vOut.NormalW = mul(vIn.NormalL, (float3x3) g_WorldInvTranspose);
	vOut.Tex = vIn.Tex;
	return vOut;
}
