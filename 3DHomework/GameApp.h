// 程序主体框架

#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "Geometry.h"
#include "LightHelper.h"
#include "Camera.h"
#include "RenderStates.h"

class GameApp : public D3DApp {
public:

	struct CBChangesEveryDrawing {
		Material material;
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldInvTranspose;
		int isShadow;
		DirectX::XMFLOAT3 pad1;
	};

	struct CBChangesEveryFrame {
		DirectX::XMMATRIX view;
		DirectX::XMFLOAT4 eyePos;
	};

	struct CBChangesOnResize {
		DirectX::XMMATRIX proj;
	};

	struct CBChangesRarely {
		DirectX::XMMATRIX shadow;
		DirectionalLight dirLight[10];
		PointLight pointLight[10];
		SpotLight spotLight[10];
		int numDirLight;
		int numPointLight;
		int numSpotLight;
		float pad;		// 打包保证16字节对齐
	};

	struct CBSkyBoxChangesEveryDrawing {
		DirectX::XMMATRIX worldViewProj;
	};

	// 一个尽可能小的游戏对象类
	class GameObject {
	public:
		GameObject();

		// 获取位置
		DirectX::XMFLOAT3 GetPosition() const;

		// 设置位置
		void SetPosition(float x, float y, float z);

		// 设置缓冲区
		template<class VertexType>
		void SetBuffer(ID3D11Device * device, const Geometry::MeshData<VertexType>& meshData);
		// 设置纹理
		void SetTexture(ID3D11ShaderResourceView * texture);
		// 设置矩阵
		void SetWorldMatrix(const DirectX::XMFLOAT4X4& world);
		void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX world);

		DirectX::XMFLOAT4X4 GetWorldMatrix() const;

		// 绘制
		void Draw(ID3D11DeviceContext * deviceContext, GameApp * gameApp);

		void SetShadowState(bool state);

	private:
		DirectX::XMFLOAT4X4 m_WorldMatrix;				    // 世界矩阵
		ComPtr<ID3D11ShaderResourceView> m_pTexture;		// 纹理
		ComPtr<ID3D11Buffer> m_pVertexBuffer;				// 顶点缓冲区
		ComPtr<ID3D11Buffer> m_pIndexBuffer;				// 索引缓冲区
		UINT m_VertexStride;								// 顶点字节大小
		UINT m_IndexCount;								    // 索引数目
		bool m_IsShadow;									// 当前是否为阴影
	};

	// 一个尽可能小的天空盒对象类
	class SkyBox {
	public:
		SkyBox();

		// 设置缓冲区
		template<class VertexType>
		void SetBuffer(ID3D11Device * device, const Geometry::MeshData<VertexType>& meshData);
		// 设置纹理
		void SetTextureCube(ID3D11ShaderResourceView * texture);
		void XM_CALLCONV SetWorldViewProjMatrix(DirectX::FXMMATRIX WVP);
		// 绘制
		void Draw(ID3D11DeviceContext * deviceContext, GameApp *gameApp, Camera & camera);

	private:
		DirectX::XMFLOAT4X4 m_worldViewProj;
		ComPtr<ID3D11ShaderResourceView> m_pTextureCube;    // 纹理
		ComPtr<ID3D11Buffer> m_pVertexBuffer;				// 顶点缓冲区
		ComPtr<ID3D11Buffer> m_pIndexBuffer;				// 索引缓冲区
		UINT m_VertexStride;								// 顶点字节大小
		UINT m_IndexCount;								    // 索引数目	
	};

	// 摄像机模式
	enum class CameraMode { FirstPerson, ThirdPerson };
	
public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

private:
	bool InitEffect();
	bool InitResource();

	void SetRenderDefault();
	void SetSkyBoxRender();
	void SetShadowRender();

private:
	
	ComPtr<ID2D1SolidColorBrush> m_pColorBrush;				    // 单色笔刷
	ComPtr<IDWriteFont> m_pFont;								// 字体
	ComPtr<IDWriteTextFormat> m_pTextFormat;					// 文本格式

	ComPtr<ID3D11InputLayout> m_pVertexLayout3D;				// 用于3D的顶点输入布局
	ComPtr<ID3D11InputLayout> m_pVertexLayoutSkyBox;		    // 用于SkyBox的顶点输入布局


	ComPtr<ID3D11Buffer> m_pConstantBuffers[5];				    // 常量缓冲区

	Material m_ShadowMat;									    // 阴影材质
	Material m_WoodCrateMat;									// 木盒材质

	GameObject m_Body;											// 车身
	GameObject m_Wheels[2];									    // 前后轮
	GameObject m_Floor;										    // 地板
	std::vector<GameObject> m_Walls;							// 墙壁

	SkyBox m_SkyBox;											// 天空盒

	ComPtr<ID3D11VertexShader> m_pVertexShader3D;				// 用于3D的顶点着色器
	ComPtr<ID3D11VertexShader> m_pVertexShaderSkyBox;			// 用于SkyBox的顶点着色器

	ComPtr<ID3D11PixelShader> m_pPixelShader3D;				    // 用于3D的像素着色器
	ComPtr<ID3D11PixelShader> m_pPixelShaderSkyBox;				// 用于SkyBox的像素着色器

	CBChangesEveryFrame m_CBFrame;							    // 该缓冲区存放仅在每一帧进行更新的变量
	CBChangesOnResize m_CBOnResize;							    // 该缓冲区存放仅在窗口大小变化时更新的变量
	CBChangesRarely m_CBRarely;								    // 该缓冲区存放不会再进行修改的变量

	std::shared_ptr<Camera> m_pCamera;						    // 摄像机
	CameraMode m_CameraMode;									// 摄像机模式
};


#endif