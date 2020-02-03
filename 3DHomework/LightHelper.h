// 定义光照相关结构体

#ifndef LIGHTHELPER_H
#define LIGHTHELPER_H

#include <cstring>
#include <DirectXMath.h>


// 方向光
struct DirectionalLight {
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;
	DirectX::XMFLOAT3 direction;
	float pad;	// 填充
};

// 点光
struct PointLight {
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;
	
	DirectX::XMFLOAT3 position;
	float range;

	DirectX::XMFLOAT3 att;
	float pad;	// 填充
};

// 聚光灯
struct SpotLight {
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;

	DirectX::XMFLOAT3 position;
	float range;

	DirectX::XMFLOAT3 direction;
	float spot;

	DirectX::XMFLOAT3 att;
	float pad; // 填充
};

// 物体表面材质
struct Material {
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular; // w = 镜面反射强度
	DirectX::XMFLOAT4 reflect;
};

#endif
