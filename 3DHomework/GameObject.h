#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "d3dApp.h"
#include "Geometry.h"


// һ��������С����Ϸ������
class GameObject
{
public:
	// ʹ��ģ�����(C++11)��������
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	GameObject();

	// ��ȡλ��
	DirectX::XMFLOAT3 GetPosition() const;

	// ����λ��
	void SetPosition(DirectX::XMFLOAT3 pos);

	// ���û�����
	template<class VertexType, class IndexType>
	void SetBuffer(ID3D11Device * device, const Geometry::MeshData<VertexType, IndexType>& meshData);
	// ��������
	void SetTexture(ID3D11ShaderResourceView * texture);
	// ���þ���
	void SetWorldMatrix(const DirectX::XMFLOAT4X4& world);
	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX world);

	DirectX::XMFLOAT4X4 GetWorldMatrix() const;

	// ����
	void Draw(ID3D11DeviceContext * deviceContext);

	// ���õ��Զ�����
	// �����������������ã����Զ�����Ҳ��Ҫ����������
	void SetDebugObjectName(const std::string& name);
private:
	DirectX::XMFLOAT4X4 m_WorldMatrix;				    // �������
	ComPtr<ID3D11ShaderResourceView> m_pTexture;		// ����
	ComPtr<ID3D11Buffer> m_pVertexBuffer;				// ���㻺����
	ComPtr<ID3D11Buffer> m_pIndexBuffer;				// ����������
	UINT m_VertexStride;								// �����ֽڴ�С
	UINT m_IndexCount;								    // ������Ŀ	
};


#endif

