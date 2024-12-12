// William Duprey
// 12/4/24
// Game Class Implementation
// Modified from starter code provided by Prof. Chris Cascioli

#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"

#include <DirectXMath.h>
#include <WICTextureLoader.h>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// ImGui includes
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Starter code has these empty scopes to organize, so here's one for ImGui setup
	{
		// Initialize ImGui itself & platform/renderer backends
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplWin32_Init(Window::Handle());
		ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());

		// Three color styles to choose from
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();
		//ImGui::StyleColorsClassic();
	}

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShadersMaterialsMeshes();
	CreateEntities();
	CreateLights();
	CreateShadowMapResources();
	CreatePostProcessResources();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// TODO: Use simple shader to set shaders here
	}

	// Initialize the background color float array
	bgColor = std::make_shared<float[]>(4);

	// A poem:
	bgColor[0] = 0.4f;	// Default to the old reliable, the tried and true
	bgColor[1] = 0.6f;	// Corn
	bgColor[2] = 0.75f;	// Flower
	bgColor[3] = 1.0f;	// Blue

	// --- Create a bunch of cameras ---
	// Standard, default camera
	cameras.push_back(std::make_shared<Camera>(
		XMFLOAT3(-0.5f, 6.25f, -15.5f),
		Window::AspectRatio()
	));
	cameras[0]->GetTransform()->SetRotation(0.366f, 0, 0);
	
	// Unmoving (unless changed with ImGui), orthographic camera
	cameras.push_back(std::make_shared<Camera>(
		XMFLOAT3(3, 1, -3),
		Window::AspectRatio(),
		XM_PIDIV4,
		0.01f,
		100.0f,
		false,			// Don't do perspective (do orthographic)
		17.5f,			// Ortho width
		0.5f			// Slow move speed
	));
	cameras[1]->GetTransform()->Rotate(0.25f, -1, 0);
	
	// Fisheye camera
	cameras.push_back(std::make_shared<Camera>(
		XMFLOAT3(0, 0, -3),
		Window::AspectRatio(),
		XM_PI - 0.1f
	));
	activeCam = cameras[0];

	moveEntities = true;
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// Cleans up ImGui memory
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	//ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


// --------------------------------------------------------
// Uses SimpleShader to set up necessary shaders, loads
// textures from files using WICTextureLoader, and creates
// some lights.
// --------------------------------------------------------
void Game::LoadShadersMaterialsMeshes()
{
	// --- Load Shaders ---
	std::shared_ptr<SimpleVertexShader> vertexShader = 
		std::make_shared<SimpleVertexShader>(
			Graphics::Device, Graphics::Context, 
			FixPath(L"VertexShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> pixelShader = 
		std::make_shared<SimplePixelShader>(
			Graphics::Device, Graphics::Context,
			FixPath(L"PixelShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> uvPS =
		std::make_shared<SimplePixelShader>(
			Graphics::Device, Graphics::Context,
			FixPath(L"uvPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> normalPS =
		std::make_shared<SimplePixelShader>(
			Graphics::Device, Graphics::Context,
			FixPath(L"normalPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> voronoi =
		std::make_shared<SimplePixelShader>(
			Graphics::Device, Graphics::Context,
			FixPath(L"Voronoi.cso").c_str());

	// Load shadow mapping vertex shader
	shadowVS = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context,
		FixPath(L"ShadowMapVS.cso").c_str());

	// --- Load textures ---
	// Bronze textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeMetalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeRoughSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/bronze_albedo.png").c_str(), 0, bronzeAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/bronze_metal.png").c_str(), 0, bronzeMetalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/bronze_normals.png").c_str(), 0, bronzeNormalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/bronze_roughness.png").c_str(),0, bronzeRoughSRV.GetAddressOf());

	// Cobblestone textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleMetalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleRoughSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/cobblestone_albedo.png").c_str(), 0, cobbleAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/cobblestone_metal.png").c_str(), 0, cobbleMetalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/cobblestone_normals.png").c_str(), 0, cobbleNormalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/cobblestone_roughness.png").c_str(),0, cobbleRoughSRV.GetAddressOf());

	// Floor textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorMetalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorRoughSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/floor_albedo.png").c_str(), 0, floorAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/floor_metal.png").c_str(), 0, floorMetalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/floor_normals.png").c_str(), 0, floorNormalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/floor_roughness.png").c_str(), 0, floorRoughSRV.GetAddressOf());

	// Paint textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintMetalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintRoughSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/paint_albedo.png").c_str(), 0, paintAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/paint_metal.png").c_str(), 0, paintMetalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/paint_normals.png").c_str(), 0, paintNormalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/paint_roughness.png").c_str(), 0, paintRoughSRV.GetAddressOf());

	// Rough textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughMetalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughRoughSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/rough_albedo.png").c_str(), 0, roughAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/rough_metal.png").c_str(), 0, roughMetalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/rough_normals.png").c_str(), 0, roughNormalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/rough_roughness.png").c_str(), 0, roughRoughSRV.GetAddressOf());

	// Scratched textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchMetalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchRoughSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/scratched_albedo.png").c_str(), 0, scratchAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/scratched_metal.png").c_str(), 0, scratchMetalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/scratched_normals.png").c_str(), 0, scratchNormalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/scratched_roughness.png").c_str(), 0, scratchRoughSRV.GetAddressOf());

	// Wood textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodAlbedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodMetalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodRoughSRV;
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/wood_albedo.png").c_str(), 0, woodAlbedoSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/wood_metal.png").c_str(), 0, woodMetalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/wood_normals.png").c_str(), 0, woodNormalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/PBR/wood_roughness.png").c_str(), 0, woodRoughSRV.GetAddressOf());

	// --- Create sampler state ---
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 8;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device->CreateSamplerState(&samplerDesc, sampler.GetAddressOf());

	// --- Create some Materials ---
	std::shared_ptr<Material> mat;
	
	// Bronze material
	mat = std::make_shared<Material>(
		"Bronze",
		XMFLOAT3(1.0f, 1.0f, 1.0f),	// White color tint
		vertexShader,				// Standard vertex shader
		pixelShader,				// Standard pixel shader
		XMFLOAT2(1.0f, 1.0f),		// Scale by 1
		XMFLOAT2(0.0f, 0.0f)		// Offset by 0
	);
	mat->AddTextureSRV("Albedo",       bronzeAlbedoSRV);
	mat->AddTextureSRV("MetalnessMap", bronzeMetalSRV);
	mat->AddTextureSRV("NormalMap",    bronzeNormalSRV);
	mat->AddTextureSRV("RoughnessMap", bronzeRoughSRV);
	mat->AddSampler("BasicSampler", sampler);
	materials.push_back(mat);
	
	// Cobblestone material
	mat = std::make_shared<Material>("Cobblestone", XMFLOAT3(1.0f, 1.0f, 1.0f), vertexShader, pixelShader, XMFLOAT2(1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f));
	mat->AddTextureSRV("Albedo",       cobbleAlbedoSRV);
	mat->AddTextureSRV("MetalnessMap", cobbleMetalSRV);
	mat->AddTextureSRV("NormalMap",    cobbleNormalSRV);
	mat->AddTextureSRV("RoughnessMap", cobbleRoughSRV);
	mat->AddSampler("BasicSampler", sampler);
	materials.push_back(mat);

	// Floor material
	mat = std::make_shared<Material>("Floor", XMFLOAT3(1.0f, 1.0f, 1.0f), vertexShader, pixelShader, XMFLOAT2(1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f));
	mat->AddTextureSRV("Albedo",       floorAlbedoSRV);
	mat->AddTextureSRV("MetalnessMap", floorMetalSRV);
	mat->AddTextureSRV("NormalMap",    floorNormalSRV);
	mat->AddTextureSRV("RoughnessMap", floorRoughSRV);
	mat->AddSampler("BasicSampler", sampler);
	materials.push_back(mat);

	// Paint material
	mat = std::make_shared<Material>("Paint", XMFLOAT3(1.0f, 1.0f, 1.0f), vertexShader, pixelShader, XMFLOAT2(1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f));
	mat->AddTextureSRV("Albedo",       paintAlbedoSRV);
	mat->AddTextureSRV("MetalnessMap", paintMetalSRV);
	mat->AddTextureSRV("NormalMap",    paintNormalSRV);
	mat->AddTextureSRV("RoughnessMap", paintRoughSRV);
	mat->AddSampler("BasicSampler", sampler);
	materials.push_back(mat);

	// Rough material
	mat = std::make_shared<Material>("Rough", XMFLOAT3(1.0f, 1.0f, 1.0f), vertexShader, pixelShader, XMFLOAT2(1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f));
	mat->AddTextureSRV("Albedo",       roughAlbedoSRV);
	mat->AddTextureSRV("MetalnessMap", roughMetalSRV);
	mat->AddTextureSRV("NormalMap",    roughNormalSRV);
	mat->AddTextureSRV("RoughnessMap", roughRoughSRV);
	mat->AddSampler("BasicSampler", sampler);
	materials.push_back(mat);

	// Scratched material
	mat = std::make_shared<Material>("Scratched", XMFLOAT3(1.0f, 1.0f, 1.0f), vertexShader, pixelShader, XMFLOAT2(1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f));
	mat->AddTextureSRV("Albedo",       scratchAlbedoSRV);
	mat->AddTextureSRV("MetalnessMap", scratchMetalSRV);
	mat->AddTextureSRV("NormalMap",    scratchNormalSRV);
	mat->AddTextureSRV("RoughnessMap", scratchRoughSRV);
	mat->AddSampler("BasicSampler", sampler);
	materials.push_back(mat);

	// Wood material
	mat = std::make_shared<Material>("Rough", XMFLOAT3(1.0f, 1.0f, 1.0f), vertexShader, pixelShader, XMFLOAT2(1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f));
	mat->AddTextureSRV("Albedo",       woodAlbedoSRV);
	mat->AddTextureSRV("MetalnessMap", woodMetalSRV);
	mat->AddTextureSRV("NormalMap",    woodNormalSRV);
	mat->AddTextureSRV("RoughnessMap", woodRoughSRV);
	mat->AddSampler("BasicSampler", sampler);
	materials.push_back(mat);

	// --- Load meshes from files ---
	meshes.push_back(std::make_shared<Mesh>("Cube",
		FixPath("../../Assets/Models/cube.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("Cylinder",
		FixPath("../../Assets/Models/cylinder.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("Helix",
		FixPath("../../Assets/Models/helix.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("Sphere",
		FixPath("../../Assets/Models/sphere.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("Torus",
		FixPath("../../Assets/Models/torus.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("Quad",
		FixPath("../../Assets/Models/quad.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("Quad Double Sided",
		FixPath("../../Assets/Models/quad_double_sided.obj").c_str()));

	// --- Set up the sky ---
	std::shared_ptr<SimpleVertexShader> skyVS =
		std::make_shared<SimpleVertexShader>(
			Graphics::Device, Graphics::Context,
			FixPath(L"SkyVS.cso").c_str());
	std::shared_ptr<SimplePixelShader> skyPS =
		std::make_shared<SimplePixelShader>(
			Graphics::Device, Graphics::Context,
			FixPath(L"SkyPS.cso").c_str());
	sky = std::make_shared<Sky>(FixPath(L"../../Assets/Textures/Skies/Planet/right.png").c_str(),
		FixPath(L"../../Assets/Textures/Skies/Planet/left.png").c_str(),
		FixPath(L"../../Assets/Textures/Skies/Planet/up.png").c_str(),
		FixPath(L"../../Assets/Textures/Skies/Planet/down.png").c_str(),
		FixPath(L"../../Assets/Textures/Skies/Planet/front.png").c_str(),
		FixPath(L"../../Assets/Textures/Skies/Planet/back.png").c_str(),
		skyVS, skyPS, meshes[0], sampler);
}

void Game::CreateEntities()
{
	// --- Create a bunch of entities ---
	// Wood floor (quad) to showcase shadows
	entities.push_back(std::make_shared<GameEntity>(
		meshes[6], materials[6]));

	// Scale the floor up
	entities[0]->GetTransform()->SetScale(20, 1, 20);

	// Various 3D shapes for fun shadow things
	entities.push_back(std::make_shared<GameEntity>(
		meshes[0], materials[0]));	// Cube
	entities.push_back(std::make_shared<GameEntity>(
		meshes[1], materials[1]));	// Cylinder
	entities.push_back(std::make_shared<GameEntity>(
		meshes[2], materials[2]));	// Helix
	entities.push_back(std::make_shared<GameEntity>(
		meshes[3], materials[3]));	// Sphere
	entities.push_back(std::make_shared<GameEntity>(
		meshes[4], materials[4]));	// Torus
	
	// Move those entities around
	entities[0]->GetTransform()->MoveAbsolute(0, -0.5f, 0);
	entities[1]->GetTransform()->MoveAbsolute(-6, 0, -1);
	entities[2]->GetTransform()->MoveAbsolute(-3, 1.5f, 1);
	entities[3]->GetTransform()->MoveAbsolute(1.5f, 1.5f, -3);
	entities[4]->GetTransform()->MoveAbsolute(1.5f, 1.5f, 0);
	entities[5]->GetTransform()->MoveAbsolute(6, 1.5f, 0);	
	entities[5]->GetTransform()->Rotate(-XM_PIDIV4, 0, 0);
}

void Game::CreateLights()
{
	// --- Create Lights ---
	//Light directional1 = {};
	//directional1.Type = LIGHT_TYPE_DIRECTIONAL;
	//directional1.Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	//directional1.Color = XMFLOAT3(1.0f, 0.0f, 0.0f);
	//directional1.Intensity = 1.0f;

	// Primary, shadow-casting light
	Light directional2 = {};
	directional2.Type = LIGHT_TYPE_DIRECTIONAL;
	directional2.Direction = XMFLOAT3(0.0f, -1.0f, 0.5f);
	directional2.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	directional2.Intensity = 1.0f;

	Light directional3 = {};
	directional3.Type = LIGHT_TYPE_DIRECTIONAL;
	directional3.Direction = XMFLOAT3(0.0f, -1.0f, -1.0f);
	directional3.Color = XMFLOAT3(0.0f, 0.0f, 0.0f);
	directional3.Intensity = 1.0f;
	
	//Light point1 = {};
	//point1.Type = LIGHT_TYPE_POINT;
	//point1.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	//point1.Intensity = 1.0f;
	//point1.Position = XMFLOAT3(-1.5f, 0, 0);
	//point1.Range = 10.0f;
	
	//Light point2 = {};
	//point2.Type = LIGHT_TYPE_POINT;
	//point2.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	//point2.Intensity = 0.5f;
	//point2.Position = XMFLOAT3(1.5f, 0, 0);
	//point2.Range = 10.0f;

	//lights.push_back(directional1);
	lights.push_back(directional2);
	lights.push_back(directional3);
	//lights.push_back(point1);
	//lights.push_back(point2);

	// Normalize directions for everything other than point lights
	for (int i = 0; i < lights.size(); i++)
	{
		if (lights[i].Type != LIGHT_TYPE_POINT) {
			XMStoreFloat3(
				&lights[i].Direction,
				XMVector3Normalize(XMLoadFloat3(&lights[i].Direction))
			);
		}
	}
}

// --------------------------------------------------------
// Helper method for setting up shadow mapping stuff.
// --------------------------------------------------------
void Game::CreateShadowMapResources()
{
	// Reset existing API objects
	shadowDSV.Reset();
	shadowSRV.Reset();
	shadowSampler.Reset();
	shadowRasterizer.Reset();

	// Shadow mapping fields
	shadowMapResolution = 1024;	// Power of 2
	lightProjectionSize = 15.0f;

	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapResolution;
	shadowDesc.Height = shadowMapResolution;
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	Graphics::Device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	// Create the depth / stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	Graphics::Device->CreateDepthStencilView(
		shadowTexture.Get(),
		&shadowDSDesc,
		shadowDSV.GetAddressOf());

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	Graphics::Device->CreateShaderResourceView(
		shadowTexture.Get(),
		&srvDesc,
		shadowSRV.GetAddressOf());

	// Create shadow rasterizer for depth biasing
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Min. precision, not world units
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; // Bias based on slope
	Graphics::Device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	// Set up sampler for comparison
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; // Only need the first component
	Graphics::Device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	// Set up light view matrix
	// Assume first light is the shadow-casting one
	XMVECTOR lightDirection = XMVector3Normalize(
		XMLoadFloat3(&lights[0].Direction));
	XMMATRIX lightView = XMMatrixLookToLH(
		-lightDirection * 20,		// Position: "Backing up" 20 units from origin
		lightDirection,				// Direction: light's direction
		XMVectorSet(0, 1, 0, 0));	// Up: World up vector (Y axis)
	XMStoreFloat4x4(&lightViewMatrix, lightView);

	// Set up light projection matrix
	XMMATRIX lightProjection = XMMatrixOrthographicLH(
		lightProjectionSize,	// Width
		lightProjectionSize,	// Height
		1.0f,					// Near plane
		100.0f);				// Far plane
	XMStoreFloat4x4(&lightProjectionMatrix, lightProjection);
}

// --------------------------------------------------------
// Helper method for setting up post process stuff.
// If the screen is resized, this needs to be re-run.
// --------------------------------------------------------
void Game::CreatePostProcessResources()
{
	// Sampler state for post processing
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device->CreateSamplerState(&ppSampDesc, ppSampler.GetAddressOf());

	// Describe the texture we're creating
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = Window::Width();
	textureDesc.Height = Window::Height();
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	// Create the resource (no need to track if after views are made)
	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppTexture;
	Graphics::Device->CreateTexture2D(&textureDesc, 0, ppTexture.GetAddressOf());
	
	// Create the render target view
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	Graphics::Device->CreateRenderTargetView(
		ppTexture.Get(),
		&rtvDesc,
		ppRTV.ReleaseAndGetAddressOf());

	// Create the shader resource view
	// By passing it a null description for the SRV,
	// it gives a default SRV with access to entire resource
	Graphics::Device->CreateShaderResourceView(
		ppTexture.Get(),
		0,
		ppSRV.ReleaseAndGetAddressOf());
}

// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Only calculate projection matrix if there's actually
	// a camera to use ( OnResize can be called before it
	// is initialized, which leads to some problems ).
	if (activeCam)
	{
		activeCam->UpdateProjectionMatrix(Window::AspectRatio());
		CreatePostProcessResources();
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Update ImGui at the start of the frame so it has fresh data
	NewFrameUI(deltaTime);
	BuildUI();

	// --- Move game entities ---
	if (moveEntities) 
	{
		entities[1]->GetTransform()->Rotate(deltaTime, 0, -deltaTime);
		entities[2]->GetTransform()->Rotate(0, 0, deltaTime);
		entities[3]->GetTransform()->SetPosition(2 + (cos(totalTime)), 2.5f, -2);
		entities[4]->GetTransform()->SetScale(1.1f + cos(totalTime), 1.1f + sin(totalTime), 1.1f + cos(totalTime));
		entities[5]->GetTransform()->Rotate(0, deltaTime, 0);
	}

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	// Update the camera last
	activeCam->Update(deltaTime);
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	// Clear the back buffer (erase what's on screen) and depth buffer
	Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(), bgColor.get());
	Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// --- Shadow map draw setup ---
	RenderShadowMap();

	// --- Draw entities ---
	for (int i = 0; i < entities.size(); ++i)
	{
		// Set vertex shader values
		std::shared_ptr<SimpleVertexShader> vs = entities[i]->GetMaterial()->GetVertexShader();
		vs->SetMatrix4x4("lightView", lightViewMatrix);
		vs->SetMatrix4x4("lightProjection", lightProjectionMatrix);

		// Set pixel shader values (these statements could probably
		// go in the Entity draw method, but I don't know)
		std::shared_ptr<SimplePixelShader> ps = entities[i]->GetMaterial()->GetPixelShader();
		ps->SetFloat("time", totalTime);
		ps->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());
		ps->SetInt("lightCount", (int)lights.size());
		ps->SetShaderResourceView("ShadowMap", shadowSRV);
		ps->SetSamplerState("ShadowSampler", shadowSampler);

		entities[i]->Draw(activeCam);
	}

	// Draw the sky after entities, as depth buffer will 
	// ensure redundant pixels are not rendered
	sky->Draw(activeCam);

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Unbind shadow map to fix D3D warnings
		// (shadow map cannot be a depth buffer 
		// and shader resource at the same time)
		ID3D11ShaderResourceView* nullSRVs[128] = {};
		Graphics::Context->PSSetShaderResources(0, 128, nullSRVs);

		// Draw ImGui as the last thing, before swapChain->Present()
		ImGui::Render(); // Turns this frame's UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen

		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}

// --------------------------------------------------------
// Helper method that handles rendering the shadow map
// for each Game Draw call.
// --------------------------------------------------------
void Game::RenderShadowMap() 
{
	// Clear shadow map
	//Graphics::Context->OMSetRenderTargets(0, 0, shadowDSV.Get());
	Graphics::Context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	Graphics::Context->RSSetState(shadowRasterizer.Get());

	// Set up the output merger state
	ID3D11RenderTargetView* nullRTV = {};
	Graphics::Context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());

	// Deactivate pixel shader (unbind it)
	Graphics::Context->PSSetShader(0, 0, 0);

	// Change viewport
	D3D11_VIEWPORT viewport = {};
	viewport.Width = (float)shadowMapResolution;
	viewport.Height = (float)shadowMapResolution;
	viewport.MaxDepth = 1.0f;
	Graphics::Context->RSSetViewports(1, &viewport);

	// Render entities
	shadowVS->SetShader();
	shadowVS->SetMatrix4x4("view", lightViewMatrix);
	shadowVS->SetMatrix4x4("projection", lightProjectionMatrix);

	// Loop and draw all entities
	for (auto& e : entities)
	{
		shadowVS->SetMatrix4x4("world", e->GetTransform()->GetWorldMatrix());
		shadowVS->CopyAllBufferData();

		// Draw the mesh directly to avoid the entity's material
		e->GetMesh()->SetBuffersAndDraw();
	}

	// Reset the pipeline
	viewport.Width = (float)Window::Width();
	viewport.Height = (float)Window::Height();
	Graphics::Context->RSSetViewports(1, &viewport);
	Graphics::Context->OMSetRenderTargets(
		1,
		Graphics::BackBufferRTV.GetAddressOf(),
		Graphics::DepthBufferDSV.Get());
	Graphics::Context->RSSetState(0);
}

///////////////////////////////////////////////////////////////////////////////
// ------------------------ UPDATE HELPER METHODS -------------------------- //
///////////////////////////////////////////////////////////////////////////////
// --------------------------------------------------------
// Updates ImGui with time, window dimensions, and input.
// Called at the start of a new frame.
// --------------------------------------------------------
void Game::NewFrameUI(float deltaTime)
{
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
}


// --------------------------------------------------------
// Creates ImGui elements for the debug demo window.
// --------------------------------------------------------
void Game::BuildUI()
{
	if (showDemoUI)
	{
		ImGui::ShowDemoWindow();
	}

	// Create a new window with the given header
	ImGui::Begin("Inspector");

	// Create a collapsible header for App Details
	if (ImGui::TreeNode("App Details"))
	{
		ImGui::Spacing();
		ImGui::Text("Framerate: %f", ImGui::GetIO().Framerate);
		ImGui::Text("Frame Time: %fms", 1000 / ImGui::GetIO().Framerate);
		ImGui::Text("Window Client Size: %dx%d", Window::Width(), Window::Height());
		ImGui::Text("Total Pixels: %d", Window::Width() * Window::Height());
		ImGui::ColorEdit4("Background Color", bgColor.get());

		// Fully admit to copying this straight from the Demo code, 
		// since it's just really nice having it so compact
		if (ImGui::Button(showDemoUI ? "Hide ImGui Demo Window" : "Show ImGui Demo Window"))
		{
			showDemoUI = !showDemoUI;
		}
		ImGui::Spacing();
		ImGui::TreePop();
	}

	// Create a collapsible header for Mesh Details
	if (ImGui::TreeNode("Meshes"))
	{
		// For every mesh, make a collapsible header
		for (int i = 0; i < meshes.size(); ++i)
		{
			ImGui::PushID(meshes[i].get());
			// Collapsible header for each mesh
			if (ImGui::TreeNode("Mesh Node", "Mesh: %s", meshes[i]->GetName()))
			{
				ImGui::Spacing();
				// Get triangle count by dividing index buffer size by 3
				ImGui::Text("Triangles: %d", (meshes[i]->GetIndexCount() / 3));
				ImGui::Text("Vertices: %d", meshes[i]->GetVertexCount());
				ImGui::Text("Indices: %d", meshes[i]->GetIndexCount());
				ImGui::Spacing();
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	// Create a collapsible header for the Game Entities
	if (ImGui::TreeNode("Game Entities"))
	{
		ImGui::Checkbox("Move Entities", &moveEntities);

		// For every entity, make a collapsible header
		for (int i = 0; i < entities.size(); ++i) 
		{
			// Push current ID so that multiple entities
			// can have the same labels
			ImGui::PushID(entities[i].get());
			if (ImGui::TreeNode("Entity Node", "Entity %d", i))
			{
				// Info for the entity's mesh
				ImGui::Text("Mesh: %s",
					entities[i].get()->GetMesh().get()->GetName());
				ImGui::Text("Material: %s",
					entities[i]->GetMaterial()->GetName());
				ImGui::Spacing();

				// Get pointer to transform and each field of it
				std::shared_ptr<Transform> trans = entities[i].get()->GetTransform();
				XMFLOAT3 pos = trans->GetPosition();
				XMFLOAT3 rot = trans->GetRotation();
				XMFLOAT3 sca = trans->GetScale();

				// Update the corresponding field of the transform
				// if it is changed in the UI
				if (ImGui::DragFloat3("Position", &pos.x, 0.01f)) {
					trans->SetPosition(pos);
				}
				if (ImGui::DragFloat3("Rotation", &rot.x, 0.01f)) {
					trans->SetRotation(rot);
				}
				if (ImGui::DragFloat3("Scale", &sca.x, 0.01f)) {
					trans->SetScale(sca);
				}

				ImGui::Spacing();
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	// Create a collapsible header for camera details
	if (ImGui::TreeNode("Camera"))
	{
		if (ImGui::TreeNode("Camera Select")) 
		{
			if (ImGui::BeginListBox("Cameras"))
			{
				for (int i = 0; i < cameras.size(); ++i)
				{
					// This feels stupid, there's 
					// probably a much better way
					if (ImGui::Selectable(
						std::string("Camera ").append(
							std::to_string(i + 1)).c_str()))
					{
						// Set the currently active camera
						activeCam = cameras[i];
					}
				}
				ImGui::EndListBox();
			}
			ImGui::TreePop();	// End of camera sleect
		}

		if (ImGui::TreeNode("Camera Details"))
		{
			// Local variables for ImGui to use
			Transform* camTrans = activeCam->GetTransform().get();
			XMFLOAT3 camPos = camTrans->GetPosition();
			XMFLOAT3 camRot = camTrans->GetRotation();
			float nearClip = activeCam->GetNearClip();
			float farClip = activeCam->GetFarClip();
			bool doingPerspective = activeCam->DoingPerspective();
			float camFov = activeCam->GetFieldOfView();
			float camOrthoWidth = activeCam->GetOrthographicWidth();

			// Controls for camera properties
			if (ImGui::DragFloat3("Position", &camPos.x, 0.01f))
			{
				camTrans->SetPosition(camPos);
			}
			if (ImGui::DragFloat3("Rotation", &camRot.x, 0.01f))
			{
				camTrans->SetRotation(camRot);
			}
			if (ImGui::DragFloat("Near Clip", &nearClip, 0.01f, 0.01f, farClip))
			{
				activeCam->SetNearClip(nearClip);
			}
			if (ImGui::DragFloat("Far Clip", &farClip, 1, nearClip, 1000))
			{
				activeCam->SetFarClip(farClip);
			}

			// Change mode of projection
			if (ImGui::Checkbox("Perspective Projection", &doingPerspective))
			{
				activeCam->SetPerspective(doingPerspective);
			}
			// If perspective, show FOV
			if (doingPerspective)
			{
				if (ImGui::DragFloat("Field of View", 
					&camFov, 0.01f, 0.1f, XM_PI))
				{
					activeCam->SetFieldOfView(camFov);
				}
			}
			// If orthographic, show orthographic width
			else
			{
				if (ImGui::DragFloat("Orthographic Width", 
					&camOrthoWidth, 0.1f, 0.1f, 20))
				{
					activeCam->SetOrthographicWidth(camOrthoWidth);
				}
			}

			ImGui::TreePop();	// End of Camera details
		}

		ImGui::TreePop();	// End of Camera header
	}

	if (ImGui::TreeNode("Lights")) 
	{
		for (int i = 0; i < lights.size(); i++)
		{
			// Push current ID so that multiple lights
			// can have the same labels
			ImGui::PushID(&lights[i]);
			if (ImGui::TreeNode("Light Node", "Light %i", i))
			{
				ImGui::ColorEdit3("Color", &lights[i].Color.x);
				ImGui::DragFloat("Intensity", &lights[i].Intensity, 0.1f, 0, 5);

				// Different controls for different light types
				switch (lights[i].Type)
				{
				case LIGHT_TYPE_DIRECTIONAL:
					ImGui::DragFloat3("Direction", &lights[i].Direction.x, 0.01f);
					break;
				case LIGHT_TYPE_POINT:
					ImGui::DragFloat3("Position", &lights[i].Position.x, 0.1f);
					ImGui::DragFloat("Range", &lights[i].Range, 0.1f, 0, 50);
					break;
				case LIGHT_TYPE_SPOT:
					// Unimplemented
					break;
				}

				ImGui::TreePop();
			}
			ImGui::PopID();
		}

		ImGui::TreePop();

	}

	// Node for shadow map debug
	if (ImGui::TreeNode("Shadow Map"))
	{
		ImGui::Image(shadowSRV.Get(), ImVec2(256, 256));
		ImGui::TreePop();
	}

	// Finish creating the ImGui Debug Window
	ImGui::End();
}
