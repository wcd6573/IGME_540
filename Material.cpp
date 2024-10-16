/*
William Duprey
10/15/24
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
Material::Material(XMFLOAT4 _colorTint, 
	std::shared_ptr<SimpleVertexShader> _vs, 
	std::shared_ptr<SimplePixelShader> _ps)
	: colorTint(_colorTint),
	  vs(_vs),
	  ps(_ps)
{
}

///////////////////////////////////////////////////////////////////////////////
// ------------------------------- GETTERS --------------------------------- //
///////////////////////////////////////////////////////////////////////////////
DirectX::XMFLOAT4 Material::GetColorTint() { return colorTint; }
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return vs; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() { return ps; }

///////////////////////////////////////////////////////////////////////////////
// ------------------------------- SETTERS --------------------------------- //
///////////////////////////////////////////////////////////////////////////////
void Material::SetColorTint(XMFLOAT4 _colorTint) { colorTint = _colorTint; }
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> _vs) { vs = _vs; }
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> _ps) { ps = _ps; }
