/*
William Duprey
11/23/24
Material Class Header
*/

#pragma once
#include <DirectXMath.h>
#include <memory>
#include <unordered_map>

#include "Camera.h"
#include "Transform.h"
#include "SimpleShader.h"


// --------------------------------------------------------
// A class representing a material for an object.
// Contains a color tint, as well as the vertex and
// pixel shaders necessary to draw whichever object
// the materrial is applied to.
// --------------------------------------------------------
class Material
{
public:
	// Constructor
	Material(const char* _name,
		DirectX::XMFLOAT3 _colorTint,
		std::shared_ptr<SimpleVertexShader> _vs,
		std::shared_ptr<SimplePixelShader> _ps,
		DirectX::XMFLOAT2 _uvScale, DirectX::XMFLOAT2 _uvOffset);

	// Getters
	const char* GetName();
	DirectX::XMFLOAT3 GetColorTint();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	DirectX::XMFLOAT2 GetUVScale();
	DirectX::XMFLOAT2 GetUVOffset();

	// Setters
	void SetColorTint(DirectX::XMFLOAT3 _colorTint);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> _vs);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> _ps);
	void SetUVScale(DirectX::XMFLOAT2 _uvScale);
	void SetUVOffset(DirectX::XMFLOAT2 _uvOffset);

	// Add methods for unordered maps
	void AddTextureSRV(std::string name, 
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(std::string name, 
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

	void PrepareMaterial(std::shared_ptr<Transform> transform,
		std::shared_ptr<Camera> camera);

private:
	const char* name;
	DirectX::XMFLOAT3 colorTint;
	float roughness;

	// Simple shader resources
	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;

	// UV modifying properties
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;

	// ComPtr hash tables / unordered_maps / dictionaries
	std::unordered_map<std::string, 
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, 
		Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
};