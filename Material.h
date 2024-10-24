/*
William Duprey
10/24/24
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
	Material(const char* _name, 
		DirectX::XMFLOAT3 _colorTint, float _roughness,
		std::shared_ptr<SimpleVertexShader> _vs,
		std::shared_ptr<SimplePixelShader> _ps);

	// Getters
	const char* GetName();
	DirectX::XMFLOAT3 GetColorTint();
	float GetRoughness();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();

	// Setters
	void SetColorTint(DirectX::XMFLOAT3 _colorTint);
	void SetRoughness(float _roughness);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> _vs);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> _ps);

private:
	const char* name;
	DirectX::XMFLOAT3 colorTint;
	float roughness;
	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;
};

