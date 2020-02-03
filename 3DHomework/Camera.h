// 提供第一人称和第三人称摄像机

#ifndef CAMERA_H
#define CAMERA_H

#include <d3d11_1.h>
#include <DirectXMath.h>

class Camera {
public:
	Camera();
	virtual ~Camera() = 0;

	// 获取摄像机位置
	DirectX::XMVECTOR GetPositionXM() const;
	DirectX::XMFLOAT3 GetPosition() const;

	// 获取矩阵
	DirectX::XMMATRIX GetViewXM() const;
	DirectX::XMMATRIX GetProjXM() const;
	DirectX::XMMATRIX GetViewProjXM() const;

	// 设置视锥体
	void SetFrustum(float fovY, float aspect, float nearZ, float farZ);

	// 设置视口
	void SetViewPort(float topLeftX, float topLeftY, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);

	// 更新观察矩阵
	virtual void UpdateViewMatrix() = 0;
protected:
	// 摄像机的观察空间坐标系对应在世界坐标系中的表示
	DirectX::XMFLOAT3 m_Position;
	DirectX::XMFLOAT3 m_Right;
	DirectX::XMFLOAT3 m_Up;
	DirectX::XMFLOAT3 m_Look;
	
	// 视锥体属性
	float m_NearZ;
	float m_FarZ;
	float m_Aspect;
	float m_FovY;
	float m_NearWindowHeight;
	float m_FarWindowHeight;

	// 观察矩阵和透视投影矩阵
	DirectX::XMFLOAT4X4 m_View;
	DirectX::XMFLOAT4X4 m_Proj;

	// 当前视口
	D3D11_VIEWPORT m_ViewPort;

};

class FirstPersonCamera : public Camera {
public:
	FirstPersonCamera();
	~FirstPersonCamera() override;

	// 设置摄像机位置
	void SetPosition(float x, float y, float z);
	// 设置摄像机的朝向
	void XM_CALLCONV LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR up);
	void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target,const DirectX::XMFLOAT3& up);
	// 上下观察
	void Pitch(float rad);
	// 左右观察
	void RotateY(float rad);

	// 更新观察矩阵
	void UpdateViewMatrix() override;
};

class ThirdPersonCamera : public Camera {
public:
	ThirdPersonCamera();
	~ThirdPersonCamera() override;

	// 绕物体垂直旋转(注意上下视野角度Phi限制在[pi/6, pi/2])
	void RotateX(float rad);
	// 绕物体水平旋转
	void RotateY(float rad);
	// 拉近物体
	void Approach(float dist);
	// 设置初始绕X轴的弧度(注意上下视野角度Phi限制在[pi/6, pi/2])
	void SetRotationX(float phi);
	// 设置初始绕Y轴的弧度
	void SetRotationY(float theta);
	// 设置并绑定待跟踪物体的位置
	void SetTarget(const DirectX::XMFLOAT3& target);
	// 设置初始距离
	void SetDistance(float dist);
	// 设置最小最大允许距离
	void SetDistanceMinMax(float minDist, float maxDist);
	// 更新观察矩阵
	void UpdateViewMatrix() override;

private:
	DirectX::XMFLOAT3 m_Target;
	float m_Distance;
	// 最小允许距离，最大允许距离
	float m_MinDist, m_MaxDist;
	// 以世界坐标系为基准，当前的旋转角度
	float m_Theta;
	float m_Phi;
};


#endif
