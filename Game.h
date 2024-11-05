// William Duprey
// 10/16/24
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
#include "Lights.h"
#include "Sky.h"
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
	void LoadShadersMaterialsMeshes();
	void CreateEntities();
	void CreateLights();

	// ImGui helper methods
	void NewFrameUI(float deltaTime);
	void BuildUI();

	// Whether the ImGui default demo window is displayed
	bool showDemoUI;

	// 4-element array of floats for holding the background color
	// TODO: Use XMFLOAT4 instead of being weird like this
	std::shared_ptr<float[]> bgColor;
	
	// Vectors of shared pointers to easily loop through these elements
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<GameEntity>> entities;
	std::vector<std::shared_ptr<Camera>> cameras;
	std::vector<std::shared_ptr<Material>> materials;
	std::vector<Light> lights;
	DirectX::XMFLOAT3 ambientColor;
	std::shared_ptr<Sky> sky;

	// One camera to rule them all
	// One camera to find them		
	// One camera to bring them all 
	// and in the darkness, bind them
	// (there's probably a pun to be made here about
	// the actual process of binding, but I'm too
	// tired to find it right now)
	std::shared_ptr<Camera> activeCam;
};

