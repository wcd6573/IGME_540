// William Duprey
// 9/5/24
// Game Class Header
// Modified from starter code provided by Prof. Chris Cascioli

#pragma once

#include <d3d11.h>
#include <wrl/client.h>

// Used for shared_ptr
#include <memory>

class Game
{
public:
	// Basic OOP setup
	Game() = default;
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Initialize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders();
	void CreateGeometry();

	// ImGui helper methods
	void NewFrameUI(float deltaTime);
	void BuildUI();
	void BuildExtraUI();	// Pointless extra UI just to experiment with ImGui
	
	// Whether the ImGui default demo window is displayed
	bool showDemoUI;

	// 4-element array of floats for holding the background color
	std::shared_ptr<float[]> bgColor;

	// Test fields for extra ImGui experimentation
	std::shared_ptr<float[]> textColor;
	bool updateTextColor;

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
};

