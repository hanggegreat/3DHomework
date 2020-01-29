#include "Model.h"
#include "d3dUtil.h"
#include "DXTrace.h"

using namespace DirectX;

Model::Model()
	: modelParts(), vertexStride()
{
}

Model::Model(ID3D11Device * device, const void* vertices, UINT vertexSize, UINT vertexCount,
	const void * indices, UINT indexCount, DXGI_FORMAT indexFormat)
	: modelParts(), vertexStride()
{
	SetMesh(device, vertices, vertexSize, vertexCount, indices, indexCount, indexFormat);
}

void Model::SetMesh(ID3D11Device * device, const void * vertices, UINT vertexSize, UINT vertexCount, const void * indices, UINT indexCount, DXGI_FORMAT indexFormat)
{
	vertexStride = vertexSize;

	modelParts.resize(1);

	modelParts[0].vertexCount = vertexCount;
	modelParts[0].indexCount = indexCount;
	modelParts[0].indexFormat = indexFormat;

	modelParts[0].material.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	modelParts[0].material.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	modelParts[0].material.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// 设置顶点缓冲区描述
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = vertexSize * vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	HR(device->CreateBuffer(&vbd, &InitData, modelParts[0].vertexBuffer.ReleaseAndGetAddressOf()));

	// 设置索引缓冲区描述
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	if (indexFormat == DXGI_FORMAT_R16_UINT)
	{
		ibd.ByteWidth = indexCount * (UINT)sizeof(WORD);
	}
	else
	{
		ibd.ByteWidth = indexCount * (UINT)sizeof(DWORD);
	}
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 新建索引缓冲区
	InitData.pSysMem = indices;
	HR(device->CreateBuffer(&ibd, &InitData, modelParts[0].indexBuffer.ReleaseAndGetAddressOf()));

}

void Model::SetDebugObjectName(const std::string& name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)

	size_t modelPartSize = modelParts.size();
	for (size_t i = 0; i < modelPartSize; ++i)
	{
		D3D11SetDebugObjectName(modelParts[i].vertexBuffer.Get(), name + ".part[" + std::to_string(i) + "].VertexBuffer");
		D3D11SetDebugObjectName(modelParts[i].indexBuffer.Get(), name + ".part[" + std::to_string(i) + "].IndexBuffer");
	}

#else
	UNREFERENCED_PARAMETER(name);
#endif
}




