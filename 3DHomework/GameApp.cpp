#include "GameApp.h"
#include "d3dUtil.h"
using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance)
	: D3DApp(hInstance),
	m_CameraMode(CameraMode::FirstPerson),
	m_CBFrame(),
	m_CBOnResize(),
	m_CBRarely()
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;

	RenderStates::InitAll(m_pd3dDevice.Get());

	if (!InitEffect())
		return false;

	if (!InitResource())
		return false;

	// 初始化鼠标，键盘不需要
	m_pMouse->SetWindow(m_hMainWnd);
	m_pMouse->SetMode(DirectX::Mouse::MODE_RELATIVE);

	return true;
}

void GameApp::OnResize()
{
	assert(m_pd2dFactory);
	assert(m_pdwriteFactory);
	// 释放D2D的相关资源
	m_pColorBrush.Reset();
	m_pd2dRenderTarget.Reset();

	D3DApp::OnResize();

	// 为D2D创建DXGI表面渲染目标
	ComPtr<IDXGISurface> surface;
	m_pSwapChain->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(surface.GetAddressOf()));
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));
	HRESULT hr = m_pd2dFactory->CreateDxgiSurfaceRenderTarget(surface.Get(), &props, m_pd2dRenderTarget.GetAddressOf());
	surface.Reset();

	if (hr == E_NOINTERFACE)
	{
		OutputDebugStringW(L"\n警告：Direct2D与Direct3D互操作性功能受限，你将无法看到文本信息。现提供下述可选方法：\n"
			L"1. 对于Win7系统，需要更新至Win7 SP1，并安装KB2670838补丁以支持Direct2D显示。\n"
			L"2. 自行完成Direct3D 10.1与Direct2D的交互。详情参阅："
			L"https://docs.microsoft.com/zh-cn/windows/desktop/Direct2D/direct2d-and-direct3d-interoperation-overview""\n"
			L"3. 使用别的字体库，比如FreeType。\n\n");
	}
	else if (hr == S_OK)
	{
		// 创建固定颜色刷和文本格式
		m_pd2dRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::White),
			m_pColorBrush.GetAddressOf());
		m_pdwriteFactory->CreateTextFormat(L"宋体", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15, L"zh-cn",
			m_pTextFormat.GetAddressOf());
	}
	else
	{
		// 报告异常问题
		assert(m_pd2dRenderTarget);
	}
	
	// 摄像机变更显示
	if (m_pCamera != nullptr)
	{
		m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
		m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
		m_CBOnResize.proj = XMMatrixTranspose(m_pCamera->GetProjXM());
		
		D3D11_MAPPED_SUBRESOURCE mappedData;
		m_pd3dImmediateContext->Map(m_pConstantBuffers[2].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
		memcpy_s(mappedData.pData, sizeof(CBChangesOnResize), &m_CBOnResize, sizeof(CBChangesOnResize));
		m_pd3dImmediateContext->Unmap(m_pConstantBuffers[2].Get(), 0);
	}
}

void GameApp::UpdateScene(float dt)
{
	// 更新鼠标事件，获取相对偏移量
	Mouse::State mouseState = m_pMouse->GetState();
	Mouse::State lastMouseState = m_MouseTracker.GetLastState();

	Keyboard::State keyState = m_pKeyboard->GetState();
	m_KeyboardTracker.Update(keyState);

	// 获取子类
	auto cam1st = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);
	auto cam3rd = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);

	static float theta = 0.0f;
	static float length = 0.0f;
	static float velocity = 0.0f;

	float frontWheelTheta = 0.0f;
	if (keyState.IsKeyDown(Keyboard::A)) {
		theta -= dt;
		frontWheelTheta -= 0.3f;
	}
	if (keyState.IsKeyDown(Keyboard::D)) {
		theta += dt;
		frontWheelTheta += 0.3f;
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::W)) {
		velocity = velocity < -0.5f ? 0.0f : 1.5f;
	}
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::S)) {
		velocity = velocity > 0.5f ? 0.0f : -1.5f;
	}

	frontWheelTheta += theta;
	XMFLOAT3 bodyPos = m_Body.GetPosition();
	float move = velocity * dt;
	float moveX = move * sin(theta);
	float moveZ = move * cos(theta);
	XMFLOAT3 frontWheelNormal(-cos(frontWheelTheta), 0.0f, sin(frontWheelTheta));
	XMFLOAT3 backWheelNormal(-cos(theta), 0.0f, sin(theta));

	length += move;

	m_Body.SetWorldMatrix(
		XMMatrixRotationY(theta)
		* XMMatrixTranslation(bodyPos.x + moveX, bodyPos.y, bodyPos.z + moveZ)
		);
	m_Wheels[0].SetWorldMatrix(
		XMMatrixRotationZ(XM_PIDIV2)
		* XMMatrixRotationY(frontWheelTheta)
		* XMMatrixRotationNormal(XMLoadFloat3(&frontWheelNormal), length)
		* XMMatrixTranslation(bodyPos.x + moveX + sin(theta), -0.49f, bodyPos.z + moveZ + cos(theta))
	);
	m_Wheels[1].SetWorldMatrix(
		XMMatrixRotationZ(XM_PIDIV2)
		* XMMatrixRotationY(theta)
		* XMMatrixRotationNormal(XMLoadFloat3(&backWheelNormal), length)
		* XMMatrixTranslation(bodyPos.x + moveX - sin(theta), -0.49f, bodyPos.z + moveZ - cos(theta))
	);

	if (m_CameraMode == CameraMode::FirstPerson)
	{
		bodyPos = m_Body.GetPosition();
		cam1st->SetPosition(bodyPos.x - 4 * sin(theta), 4.0f, bodyPos.z - 4 * cos(theta));

		// 视野旋转，防止开始的差值过大导致的突然旋转
		cam1st->Pitch(mouseState.y * dt * 1.25f);
		cam1st->RotateY(mouseState.x * dt * 1.25f);
	}
	else if (m_CameraMode == CameraMode::ThirdPerson)
	{
		// 第三人称摄像机的操作
		cam3rd->SetTarget(m_Body.GetPosition());

		// 绕物体旋转
		cam3rd->RotateX(mouseState.y * dt * 1.25f);
		cam3rd->RotateY(mouseState.x * dt * 1.25f);
		cam3rd->Approach(-mouseState.scrollWheelValue / 120 * 1.0f);
	}

	// 更新观察矩阵
	m_pCamera->UpdateViewMatrix();
	XMStoreFloat4(&m_CBFrame.eyePos, m_pCamera->GetPositionXM());
	m_CBFrame.view = XMMatrixTranspose(m_pCamera->GetViewXM());

	// 重置滚轮值
	m_pMouse->ResetScrollWheelValue();
	
	// 摄像机模式切换
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::D1) && m_CameraMode != CameraMode::FirstPerson)
	{
		if (!cam1st)
		{
			cam1st.reset(new FirstPersonCamera);
			cam1st->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = cam1st;
		}

		bodyPos = m_Body.GetPosition();
		cam1st->LookAt(XMFLOAT3(bodyPos.x - 3 * sin(theta), 3.0f, bodyPos.z - 3 * cos(theta)),
			XMFLOAT3(0.0f, 0.0f, 1.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f));

		m_CameraMode = CameraMode::FirstPerson;
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D2) && m_CameraMode != CameraMode::ThirdPerson)
	{
		if (!cam3rd)
		{
			cam3rd.reset(new ThirdPersonCamera);
			cam3rd->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = cam3rd;
		}
		XMFLOAT3 target = m_Body.GetPosition();
		cam3rd->SetTarget(target);
		cam3rd->SetDistance(8.0f);
		cam3rd->SetDistanceMinMax(3.0f, 20.0f);
		
		m_CameraMode = CameraMode::ThirdPerson;
	}
	// 退出程序，这里应向窗口发送销毁信息
	if (keyState.IsKeyDown(Keyboard::Escape))
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);
	
	D3D11_MAPPED_SUBRESOURCE mappedData;
	m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	memcpy_s(mappedData.pData, sizeof(CBChangesEveryFrame), &m_CBFrame, sizeof(CBChangesEveryFrame));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);
}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);

	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// 绘制几何模型
	m_Body.Draw(m_pd3dImmediateContext.Get(), this);
	m_Wheels[0].Draw(m_pd3dImmediateContext.Get(), this);
	m_Wheels[1].Draw(m_pd3dImmediateContext.Get(), this);
	m_Floor.Draw(m_pd3dImmediateContext.Get(), this);
	for (auto& wall : m_Walls)
		wall.Draw(m_pd3dImmediateContext.Get(), this);

	m_Body.SetShadowState(true);
	m_Wheels[0].SetShadowState(true);
	m_Wheels[1].SetShadowState(true);
	
	m_Body.Draw(m_pd3dImmediateContext.Get(), this);
	m_Wheels[0].Draw(m_pd3dImmediateContext.Get(), this);
	m_Wheels[1].Draw(m_pd3dImmediateContext.Get(), this);

	m_Body.SetShadowState(false);
	m_Wheels[0].SetShadowState(false);
	m_Wheels[1].SetShadowState(false);
	
	m_SkyBox.Draw(m_pd3dImmediateContext.Get(), this, *m_pCamera);

	// 绘制Direct2D部分
	if (m_pd2dRenderTarget != nullptr)
	{
		m_pd2dRenderTarget->BeginDraw();
		std::wstring text = L"切换摄像机模式: 1-第一人称 2-第三人称\n"
			L"W/S/A/D 前进/后退/左平移/右平移   Esc退出\n"
			L"鼠标移动控制视野 滚轮控制第三人称观察距离\n"
			L"当前模式: ";
		if (m_CameraMode == CameraMode::FirstPerson)
			text += L"第一人称";
		else if (m_CameraMode == CameraMode::ThirdPerson)
			text += L"第三人称";
		m_pd2dRenderTarget->DrawTextW(text.c_str(), (UINT32)text.length(), m_pTextFormat.Get(),
			D2D1_RECT_F{ 0.0f, 0.0f, 600.0f, 200.0f }, m_pColorBrush.Get());
		m_pd2dRenderTarget->EndDraw();
	}

	m_pSwapChain->Present(0, 0);
}


bool GameApp::InitEffect()
{
	ComPtr<ID3DBlob> blob;

	// 创建顶点着色器(天空盒)
	CreateShaderFromFile(L"HLSL\\Sky_VS.cso", L"HLSL\\Sky_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf());
	m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShaderSkyBox.GetAddressOf());
	// 创建顶点布局(天空盒)
	m_pd3dDevice->CreateInputLayout(VertexPos::inputLayout, ARRAYSIZE(VertexPos::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayoutSkyBox.GetAddressOf());
	// 创建像素着色器(天空盒)
	CreateShaderFromFile(L"HLSL\\Sky_PS.cso", L"HLSL\\Sky_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf());
	m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShaderSkyBox.GetAddressOf());


	// 创建顶点着色器(3D)
	CreateShaderFromFile(L"HLSL\\Basic_VS.cso", L"HLSL\\Basic_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf());
	m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader3D.GetAddressOf());
	// 创建顶点布局(3D)
	m_pd3dDevice->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout3D.GetAddressOf());
	// 创建像素着色器(3D)
	CreateShaderFromFile(L"HLSL\\Basic_PS.cso", L"HLSL\\Basic_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf());
	m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader3D.GetAddressOf());

	return true;
}

bool GameApp::InitResource()
{
	// ******************
	// 设置常量缓冲区描述
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// 新建用于VS和PS的常量缓冲区
	cbd.ByteWidth = sizeof(CBChangesEveryDrawing);
	m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[0].GetAddressOf());
	cbd.ByteWidth = sizeof(CBChangesEveryFrame);
	m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[1].GetAddressOf());
	cbd.ByteWidth = sizeof(CBChangesOnResize);
	m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[2].GetAddressOf());
	cbd.ByteWidth = sizeof(CBChangesRarely);
	m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[3].GetAddressOf());
	cbd.ByteWidth = sizeof(CBSkyBoxChangesEveryDrawing);
	m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[4].GetAddressOf());
	
	// ******************
	// 初始化游戏对象

	m_WoodCrateMat.ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_WoodCrateMat.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_WoodCrateMat.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 16.0f);

	m_ShadowMat.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_ShadowMat.diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	m_ShadowMat.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);

	ComPtr<ID3D11ShaderResourceView> texture;
	// 初始化天空盒
	CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\sky.dds", nullptr, texture.GetAddressOf());
	m_SkyBox.SetBuffer(m_pd3dDevice.Get(), Geometry::CreateSphere<VertexPos>(5000.0f));
	m_SkyBox.SetTextureCube(texture.Get());

	// 初始化车身
	CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\WoodCrate.dds", nullptr, texture.GetAddressOf());
	m_Body.SetBuffer(m_pd3dDevice.Get(), Geometry::CreateBox(1.5f, 1.5f, 4.0f));
	m_Body.SetTexture(texture.Get());

	m_Wheels[0].SetBuffer(m_pd3dDevice.Get(), Geometry::CreateCylinder(0.5f));
	m_Wheels[0].SetTexture(texture.Get());
	m_Wheels[0].SetWorldMatrix(XMMatrixRotationZ(XM_PIDIV2) * XMMatrixTranslation(0.0f, -0.49f, 1.0f));

	m_Wheels[1].SetBuffer(m_pd3dDevice.Get(), Geometry::CreateCylinder(0.5f));
	m_Wheels[1].SetTexture(texture.Get());
	m_Wheels[1].SetWorldMatrix(XMMatrixRotationZ(XM_PIDIV2) * XMMatrixTranslation(0.0f, -0.49f, -1.0f));
	
	// 初始化地板
	CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\floor.dds", nullptr, texture.ReleaseAndGetAddressOf());
	m_Floor.SetBuffer(m_pd3dDevice.Get(),
		Geometry::CreatePlane(XMFLOAT2(20.0f, 20.0f), XMFLOAT2(5.0f, 5.0f)));
	m_Floor.SetTexture(texture.Get());
	m_Floor.SetWorldMatrix(XMMatrixTranslation(0.0f, -1.0f, 0.0f));
	
	// 初始化墙体
	m_Walls.resize(4);
	CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\brick.dds", nullptr, texture.ReleaseAndGetAddressOf());
	// 这里控制墙体四个面的生成
	for (int i = 0; i < 4; ++i)
	{
		m_Walls[i].SetBuffer(m_pd3dDevice.Get(),
			Geometry::CreatePlane(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 1.5f)));
		XMMATRIX world = XMMatrixRotationX(-XM_PIDIV2) * XMMatrixRotationY(XM_PIDIV2 * i)
			* XMMatrixTranslation(i % 2 ? -10.0f * (i - 2) : 0.0f, 3.0f, i % 2 == 0 ? -10.0f * (i - 1) : 0.0f);
		m_Walls[i].SetWorldMatrix(world);
		m_Walls[i].SetTexture(texture.Get());
	}
		
	// ******************
	// 初始化常量缓冲区的值
	// 初始化每帧可能会变化的值
	m_CameraMode = CameraMode::FirstPerson;
	auto camera = std::shared_ptr<FirstPersonCamera>(new FirstPersonCamera);
	m_pCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	camera->LookAt(XMFLOAT3(0.0f, 4.0f, -4.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	// 初始化仅在窗口大小变动时修改的值
	m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
	m_CBOnResize.proj = XMMatrixTranspose(m_pCamera->GetProjXM());

	// 初始化不会变化的值
	// 环境光
	m_CBRarely.dirLight[0].ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.dirLight[0].diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_CBRarely.dirLight[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.dirLight[0].direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	// 灯光
	m_CBRarely.pointLight[0].position = XMFLOAT3(0.0f, 10.0f, 0.0f);
	m_CBRarely.pointLight[0].ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.pointLight[0].diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_CBRarely.pointLight[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_CBRarely.pointLight[0].att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	m_CBRarely.pointLight[0].range = 25.0f;
	m_CBRarely.numDirLight = 1;
	m_CBRarely.numPointLight = 1;
	m_CBRarely.numSpotLight = 0;
	m_CBRarely.shadow = XMMatrixTranspose(XMMatrixShadow(XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f), XMVectorSet(0.0f, 10.0f, -10.0f, 1.0f)));

	// 更新不容易被修改的常量缓冲区资源
	D3D11_MAPPED_SUBRESOURCE mappedData;
	m_pd3dImmediateContext->Map(m_pConstantBuffers[2].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	memcpy_s(mappedData.pData, sizeof(CBChangesOnResize), &m_CBOnResize, sizeof(CBChangesOnResize));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[2].Get(), 0);

	m_pd3dImmediateContext->Map(m_pConstantBuffers[3].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	memcpy_s(mappedData.pData, sizeof(CBChangesRarely), &m_CBRarely, sizeof(CBChangesRarely));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[3].Get(), 0);

	// ******************
	// 给渲染管线各个阶段绑定好所需资源
	// 设置图元类型，设定输入布局
	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// 预先绑定各自所需的缓冲区，其中每帧更新的缓冲区需要绑定到两个缓冲区上
	m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
	m_pd3dImmediateContext->VSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
	m_pd3dImmediateContext->VSSetConstantBuffers(2, 1, m_pConstantBuffers[2].GetAddressOf());
	m_pd3dImmediateContext->VSSetConstantBuffers(3, 1, m_pConstantBuffers[3].GetAddressOf());
	m_pd3dImmediateContext->VSSetConstantBuffers(4, 1, m_pConstantBuffers[4].GetAddressOf());

	m_pd3dImmediateContext->PSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
	m_pd3dImmediateContext->PSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
	m_pd3dImmediateContext->PSSetConstantBuffers(3, 1, m_pConstantBuffers[3].GetAddressOf());
	m_pd3dImmediateContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());

	return true;
}

void GameApp::SetRenderDefault()
{
	m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout3D.Get());
	m_pd3dImmediateContext->VSSetShader(m_pVertexShader3D.Get(), nullptr, 0);
	m_pd3dImmediateContext->RSSetState(nullptr);
	m_pd3dImmediateContext->PSSetShader(m_pPixelShader3D.Get(), nullptr, 0);

	// 使用各向异性过滤获取更好的绘制质量
	m_pd3dImmediateContext->PSSetSamplers(0, 1, RenderStates::SSAnistropicWrap.GetAddressOf());
	m_pd3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
	m_pd3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void GameApp::SetSkyBoxRender()
{
	m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayoutSkyBox.Get());
	m_pd3dImmediateContext->VSSetShader(m_pVertexShaderSkyBox.Get(), nullptr, 0);
	m_pd3dImmediateContext->PSSetShader(m_pPixelShaderSkyBox.Get(), nullptr, 0);

	m_pd3dImmediateContext->RSSetState(RenderStates::RSNoCull.Get());

	m_pd3dImmediateContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	m_pd3dImmediateContext->OMSetDepthStencilState(RenderStates::DSSLessEqual.Get(), 0);
	m_pd3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void GameApp::SetShadowRender()
{
	m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout3D.Get());
	m_pd3dImmediateContext->VSSetShader(m_pVertexShader3D.Get(), nullptr, 0);
	m_pd3dImmediateContext->RSSetState(RenderStates::RSNoCull.Get());
	m_pd3dImmediateContext->PSSetShader(m_pPixelShader3D.Get(), nullptr, 0);
	m_pd3dImmediateContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	m_pd3dImmediateContext->OMSetDepthStencilState(RenderStates::DSSNoDoubleBlend.Get(), 0);
	m_pd3dImmediateContext->OMSetBlendState(RenderStates::BSTransparent.Get(), nullptr, 0xFFFFFFFF);
}

GameApp::GameObject::GameObject()
	: m_IndexCount(), m_VertexStride(), m_IsShadow()
{
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());
}

DirectX::XMFLOAT3 GameApp::GameObject::GetPosition() const
{
	return XMFLOAT3(m_WorldMatrix(3, 0), m_WorldMatrix(3, 1), m_WorldMatrix(3, 2));
}

void GameApp::GameObject::SetPosition(float x, float y, float z)
{
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixTranslation(x, y, z));
}

template<class VertexType, class IndexType>
void GameApp::GameObject::SetBuffer(ID3D11Device * device, const Geometry::MeshData<VertexType, IndexType>& meshData)
{
	// 释放旧资源
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();

	// 设置顶点缓冲区描述
	m_VertexStride = sizeof(VertexType);
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = (UINT)meshData.vertexVec.size() * m_VertexStride;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = meshData.vertexVec.data();
	device->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf());


	// 设置索引缓冲区描述
	m_IndexCount = (UINT)meshData.indexVec.size();
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = m_IndexCount * sizeof(IndexType);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 新建索引缓冲区
	InitData.pSysMem = meshData.indexVec.data();
	device->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf());
}

void GameApp::GameObject::SetTexture(ID3D11ShaderResourceView * texture)
{
	m_pTexture = texture;
}

void GameApp::GameObject::SetWorldMatrix(const XMFLOAT4X4 & world)
{
	m_WorldMatrix = world;
}

DirectX::XMFLOAT4X4 GameApp::GameObject::GetWorldMatrix() const {
	return m_WorldMatrix;
}

void XM_CALLCONV GameApp::GameObject::SetWorldMatrix(FXMMATRIX world)
{
	XMStoreFloat4x4(&m_WorldMatrix, world);
}

void GameApp::GameObject::Draw(ID3D11DeviceContext * deviceContext, GameApp * gameApp)
{
	// 设置顶点/索引缓冲区
	UINT strides = m_VertexStride;
	UINT offsets = 0;

	if (m_IsShadow) {
		gameApp->SetShadowRender();
	}
	else {
		gameApp->SetRenderDefault();
	}

	deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &strides, &offsets);
	deviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// 获取之前已经绑定到渲染管线上的常量缓冲区并进行修改
	ComPtr<ID3D11Buffer> cBuffer = nullptr;
	deviceContext->VSGetConstantBuffers(0, 1, cBuffer.GetAddressOf());
	CBChangesEveryDrawing cbDrawing;

	// 内部进行转置，这样外部就不需要提前转置了
	XMMATRIX W = XMLoadFloat4x4(&m_WorldMatrix);
	cbDrawing.world = XMMatrixTranspose(W);
	cbDrawing.worldInvTranspose = XMMatrixInverse(nullptr, W);	// 两次转置抵消
	cbDrawing.isShadow = m_IsShadow;
	cbDrawing.material = m_IsShadow ? gameApp->m_ShadowMat : gameApp->m_WoodCrateMat;

	// 更新常量缓冲区
	D3D11_MAPPED_SUBRESOURCE mappedData;
	deviceContext->Map(cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	memcpy_s(mappedData.pData, sizeof(CBChangesEveryDrawing), &cbDrawing, sizeof(CBChangesEveryDrawing));
	deviceContext->Unmap(cBuffer.Get(), 0);

	// 设置纹理
	deviceContext->PSSetShaderResources(0, 1, m_pTexture.GetAddressOf());
	// 可以开始绘制
	deviceContext->DrawIndexed(m_IndexCount, 0, 0);
}

void GameApp::GameObject::SetShadowState(bool state)
{
	m_IsShadow = state;
}

GameApp::SkyBox::SkyBox() : m_VertexStride(), m_IndexCount()
{
}

void GameApp::SkyBox::SetTextureCube(ID3D11ShaderResourceView * textureCube)
{
	m_pTextureCube = textureCube;
}

void XM_CALLCONV GameApp::SkyBox::SetWorldViewProjMatrix(DirectX::FXMMATRIX WVP)
{
	XMStoreFloat4x4(&m_worldViewProj, WVP);
}

void GameApp::SkyBox::Draw(ID3D11DeviceContext * deviceContext, GameApp * gameApp, Camera & camera)
{
	// 设置顶点/索引缓冲区
	UINT strides = sizeof(XMFLOAT3);
	UINT offsets = 0;

	gameApp->SetSkyBoxRender();

	deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &strides, &offsets);
	deviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	XMFLOAT3 pos = camera.GetPosition();
	SetWorldViewProjMatrix(XMMatrixTranslation(pos.x, pos.y, pos.z) * camera.GetViewProjXM());
	SetTextureCube(m_pTextureCube.Get());

	// 获取之前已经绑定到渲染管线上的常量缓冲区并进行修改
	ComPtr<ID3D11Buffer> cBuffer = nullptr;
	deviceContext->VSGetConstantBuffers(0, 1, cBuffer.GetAddressOf());
	CBSkyBoxChangesEveryDrawing cbDrawing;

	// 内部进行转置，这样外部就不需要提前转置了
	XMMATRIX WVP = XMLoadFloat4x4(&m_worldViewProj);
	cbDrawing.worldViewProj = XMMatrixTranspose(WVP);

	// 更新常量缓冲区
	D3D11_MAPPED_SUBRESOURCE mappedData;
	deviceContext->Map(cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	memcpy_s(mappedData.pData, sizeof(CBChangesEveryDrawing), &cbDrawing, sizeof(CBChangesEveryDrawing));
	deviceContext->Unmap(cBuffer.Get(), 0);

	// 设置纹理
	deviceContext->PSSetShaderResources(1, 1, m_pTextureCube.GetAddressOf());
	// 可以开始绘制
	deviceContext->DrawIndexed(m_IndexCount, 0, 0);
}

template<class VertexType, class IndexType>
void GameApp::SkyBox::SetBuffer(ID3D11Device * device, const Geometry::MeshData<VertexType, IndexType>& meshData)
{
	// 释放旧资源
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();

	// 设置顶点缓冲区描述
	m_VertexStride = sizeof(VertexType);
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = (UINT)meshData.vertexVec.size() * m_VertexStride;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = meshData.vertexVec.data();
	device->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf());


	// 设置索引缓冲区描述
	m_IndexCount = (UINT)meshData.indexVec.size();
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = m_IndexCount * sizeof(IndexType);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 新建索引缓冲区
	InitData.pSysMem = meshData.indexVec.data();
	device->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf());
}