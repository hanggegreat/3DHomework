#include "Basic.hlsli"

// ������ɫ��
VertexPosHWNormalTex VS(VertexPosNormalTex vIn)
{
	VertexPosHWNormalTex vOut;

	matrix viewProj = mul(g_View, g_Proj);
	float4 posW = mul(float4(vIn.PosL, 1.0f), g_World);
	// ����ǰ�ڻ�����Ӱ���Ƚ���ͶӰ����
	[flatten]
	if (g_IsShadow)
	{
		posW = mul(posW, g_Shadow);
	}

	vOut.PosH = mul(posW, viewProj);
	vOut.PosW = posW.xyz;
	vOut.NormalW = mul(vIn.NormalL, (float3x3) g_WorldInvTranspose);
	vOut.Tex = vIn.Tex;
	return vOut;
}