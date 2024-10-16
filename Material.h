/*
William Duprey
10/15/24
Material Class Header
*/

#pragma once
#include <DirectXMath.h>
#include <memory>
#include "SimpleShader.h"


// --------------------------------------------------------
// A class representing a material for an object.
// COntains a color tint, as well as the vertex and
// pixel shaders necessary to draw whichever object
// the materrial is applied to.
// --------------------------------------------------------
class Material
{
public:
	// Constructor
	Material(DirectX::XMFLOAT4 _colorTint, 
		std::shared_ptr<SimpleVertexShader> _vs,
		std::shared_ptr<SimplePixelShader> _ps);

	// Getters
	DirectX::XMFLOAT4 GetColorTint();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();

	// Setters
	void SetColorTint(DirectX::XMFLOAT4 _colorTint);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> _vs);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> _ps);

private:
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;
};

