// 提供一些渲染状态

#ifndef RENDERSTATES_H
#define RENDERSTATES_H

#include <wrl/client.h>
#include <d3d11_1.h>

using namespace Microsoft::WRL;


class RenderStates {
public:
	static void InitAll(ID3D11Device * device);

public:
	static ComPtr<ID3D11RasterizerState> RSNoCull;			            // 光栅化器状态：无背面裁剪模式

	static ComPtr<ID3D11SamplerState> SSLinearWrap;			            // 采样器状态：线性过滤
	static ComPtr<ID3D11SamplerState> SSAnistropicWrap;		            // 采样器状态：各项异性过滤

	static ComPtr<ID3D11BlendState> BSTransparent;						// 混合状态：透明混合

	static ComPtr<ID3D11DepthStencilState> DSSLessEqual;		        // 深度/模板状态：允许绘制深度值相等的像素
	static ComPtr<ID3D11DepthStencilState> DSSNoDoubleBlend;	        // 深度/模板状态：无二次混合区域
};



#endif
