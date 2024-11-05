/*
William Duprey
11/5/24
Sky Header
*/

#pragma once
#include "Mesh.h"
#include "SimpleShader.h"
#include "Camera.h"

#include <memory>
#include <wrl/client.h> // Used for ComPtr

class Sky
{
public:
	Sky(const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back,
		std::shared_ptr<SimpleVertexShader> _skyVS,
		std::shared_ptr<SimplePixelShader> _skyPS,
		std::shared_ptr<Mesh> _skyMesh,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerOptions);
	
	void Draw(std::shared_ptr<Camera> cam);

private:
	// ComPtrs for ID3D11 resources needed to draw the sky
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skySRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> skyDepthState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> skyRasterState;
	
	std::shared_ptr<Mesh> skyMesh;	// Probably a cube
	std::shared_ptr<SimpleVertexShader> skyVS;
	std::shared_ptr<SimplePixelShader> skyPS;


	// --------------------------------------------------------
	// Author: Chris Cascioli
	// Purpose: Creates a cube map on the GPU from 6 individual textures
	// 
	// - You are allowed to directly copy/paste this into your code base
	//   for assignments, given that you clearly cite that this is not
	//   code of your own design.
	//
	// - Note: This code assumes you’re putting the function in Sky.cpp, 
	//   you’ve included WICTextureLoader.h and you have an ID3D11Device 
	//   ComPtr called “device”.  Make any adjustments necessary for
	//   your own implementation.
	// --------------------------------------------------------
	// Helper for creating a cubemap from 6 individual textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);
};