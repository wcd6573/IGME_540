// William Duprey
// 9/12/24
// Game Class Header
// Modified from starter code provided by Prof. Chris Cascioli

#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <vector>

#include "Mesh.h"
#include "GameEntity.h"
#include "Camera.h"
#include "Material.h"
#include "SimpleShader.h"

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
	void CreateMaterials();
	void CreateGeometry();

	// ImGui helper methods
	void NewFrameUI(float deltaTime);
	void BuildUI();

	// Whether the ImGui default demo window is displayed
	bool showDemoUI;

	// 4-element array of floats for holding the background color
	std::shared_ptr<float[]> bgColor;

	// Variables used to change the constant buffer struct using ImGui
	std::shared_ptr<float[]> offset;
	std::shared_ptr<float[]> colorTint;

	// Vectors of shared pointers to easily loop through these elements
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<GameEntity>> entities;
	std::vector<std::shared_ptr<Camera>> cameras;
	std::vector<std::shared_ptr<Material>> materials;

	// One camera to rule them all
	// One camera to find them		
	// One camera to bring them all 
	// and in the darkness, bind them
	// (there's probably a pun to be made here about
	// the actual process of binding, but I'm too
	// tired to find it right now)
	std::shared_ptr<Camera> activeCam;

	// SimpleShader pointers for current shaders
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
};

