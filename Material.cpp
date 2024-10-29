/*
William Duprey
10/24/24
Material Class Implementation
*/

#include "Material.h"
using namespace DirectX;

///////////////////////////////////////////////////////////////////////////////
// --------------------------- MATERIAL CLASS ------------------------------ //
///////////////////////////////////////////////////////////////////////////////
// --------------------------------------------------------
// Constructor for a Material. Sets up its fields,
// as constructors often do. Uses an initializer list.
// --------------------------------------------------------
Material::Material(const char* _name,
	XMFLOAT3 _colorTint, float _roughness,
	std::shared_ptr<SimpleVertexShader> _vs, 
	std::shared_ptr<SimplePixelShader> _ps)
	: name(_name),
	  colorTint(_colorTint),
	  roughness(_roughness),
	  vs(_vs),
	  ps(_ps)
{
}

///////////////////////////////////////////////////////////////////////////////
// ------------------------------- GETTERS --------------------------------- //
///////////////////////////////////////////////////////////////////////////////
const char* Material::GetName() { return name; }
DirectX::XMFLOAT3 Material::GetColorTint() { return colorTint; }
float Material::GetRoughness() { return roughness; }
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return vs; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return ps; }

///////////////////////////////////////////////////////////////////////////////
// ------------------------------- SETTERS --------------------------------- //
///////////////////////////////////////////////////////////////////////////////
void Material::SetColorTint(XMFLOAT3 _colorTint) { colorTint = _colorTint; }
void Material::SetRoughness(float _roughness) { roughness = _roughness; }
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> _vs) { vs = _vs; }
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> _ps) { ps = _ps; }

///////////////////////////////////////////////////////////////////////////////
// ----------------------- UNORDERED MAP FUNCTIONS ------------------------- //
///////////////////////////////////////////////////////////////////////////////
void Material::AddTextureSRV(std::string name, 
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({ name, srv });
}

void Material::AddSampler(std::string name, 
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({ name, sampler });
}
