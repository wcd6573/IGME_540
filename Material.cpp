/*
William Duprey
11/23/24
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
	XMFLOAT3 _colorTint,
	std::shared_ptr<SimpleVertexShader> _vs, 
	std::shared_ptr<SimplePixelShader> _ps,
	XMFLOAT2 _uvScale, XMFLOAT2 _uvOffset)
	: name(_name),
	  colorTint(_colorTint),
	  vs(_vs),
	  ps(_ps),
	  uvScale(_uvScale),
	  uvOffset(_uvOffset)
{
}

// --------------------------------------------------------
// Sets shader values and resources 
// in preparation for being drawn.
// --------------------------------------------------------
void Material::PrepareMaterial(std::shared_ptr<Transform> transform,
	std::shared_ptr<Camera> camera)
{
	// Activate the correct shaders
	vs->SetShader();
	ps->SetShader();

	// Strings must exactly match variable names in shader cbuffer
	vs->SetMatrix4x4("world", transform->GetWorldMatrix());
	vs->SetMatrix4x4("worldInvTranspose", transform->GetWorldInverseTransposeMatrix());
	vs->SetMatrix4x4("view", camera->GetViewMatrix());
	vs->SetMatrix4x4("projection", camera->GetProjectionMatrix());

	// Copy data to the GPU
	vs->CopyAllBufferData();

	// Do the same for the pixel shader
	ps->SetFloat3("colorTint", colorTint);
	ps->SetFloat3("cameraPosition", camera->GetTransform()->GetPosition());
	ps->SetFloat2("uvScale", uvScale);
	ps->SetFloat2("uvOffset", uvOffset);
	ps->CopyAllBufferData();

	// Loop through srv and sampler unordered maps to set resources
	for (auto& t : textureSRVs)
	{
		ps->SetShaderResourceView(t.first.c_str(), t.second);
	}
	for (auto& s : samplers)
	{
		ps->SetSamplerState(s.first.c_str(), s.second);
	}
}

///////////////////////////////////////////////////////////////////////////////
// ------------------------------- GETTERS --------------------------------- //
///////////////////////////////////////////////////////////////////////////////
const char* Material::GetName() { return name; }
DirectX::XMFLOAT3 Material::GetColorTint() { return colorTint; }
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return vs; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return ps; }
DirectX::XMFLOAT2 Material::GetUVScale() { return uvScale; }
DirectX::XMFLOAT2 Material::GetUVOffset() { return uvOffset; }

///////////////////////////////////////////////////////////////////////////////
// ------------------------------- SETTERS --------------------------------- //
///////////////////////////////////////////////////////////////////////////////
void Material::SetColorTint(XMFLOAT3 _colorTint) { colorTint = _colorTint; }
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> _vs) { vs = _vs; }
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> _ps) { ps = _ps; }
void Material::SetUVScale(DirectX::XMFLOAT2 _uvScale) { uvScale = _uvScale; }
void Material::SetUVOffset(DirectX::XMFLOAT2 _uvOffset) { uvOffset = _uvOffset; }

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