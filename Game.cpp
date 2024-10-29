// William Duprey
// 10/29/24
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
	LoadShadersAndCreateMaterials();
	CreateGeometry();

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

	// Set ambient color of light
	ambientColor = XMFLOAT3(0.1f, 0.1f, 0.25f);

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
void Game::LoadShadersAndCreateMaterials()
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


	// --- Load some textures ---
	// Shader Resource View ComPtrs for each texture
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brokenTilesSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brokenTilesSpecSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rustyMetalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rustyMetalSpecSRV;

	// Load textures
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/brokentiles.png").c_str(), 
		0, brokenTilesSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/brokentiles_specular.png").c_str(),
		0, brokenTilesSpecSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/rustymetal.png").c_str(),
		0, rustyMetalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/rustymetal_specular.png").c_str(),
		0, rustyMetalSpecSRV.GetAddressOf());

	// Create sampler state
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
	
	// Broken tiles material
	mat = std::make_shared<Material>(
		"Broken Tiles",
		XMFLOAT3(1.0f, 1.0f, 1.0f), 0.02f,
		vertexShader,
		pixelShader,
		XMFLOAT2(2, 2), XMFLOAT2(0, 0.5f));
	mat->AddTextureSRV("SurfaceTexture", brokenTilesSRV);
	mat->AddTextureSRV("SpecularMap", brokenTilesSpecSRV);
	mat->AddSampler("BasicSampler", sampler);
	materials.push_back(mat);
	// Rusty metal material
	mat = std::make_shared<Material>(
		"Rusty Metal",
		XMFLOAT3(1.0f, 1.0f, 1.0f), 0.5f,
		vertexShader,
		pixelShader,
		XMFLOAT2(0.75f, 2), XMFLOAT2(0.5f, 5));
	mat->AddTextureSRV("SurfaceTexture", rustyMetalSRV);
	mat->AddTextureSRV("SpecularMap", rustyMetalSpecSRV);
	mat->AddSampler("BasicSampler", sampler);
	materials.push_back(mat);

	
	// --- Create Lights ---
	Light directional1 = {};
	directional1.Type = LIGHT_TYPE_DIRECTIONAL;
	directional1.Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	directional1.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	directional1.Intensity = 1.0f;

	Light directional2 = {};
	directional2.Type = LIGHT_TYPE_DIRECTIONAL;
	directional2.Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	directional2.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	directional2.Intensity = 1.0f;

	Light directional3 = {};
	directional3.Type = LIGHT_TYPE_DIRECTIONAL;
	directional3.Direction = XMFLOAT3(-1.0f, 1.0f, -0.5f);
	directional3.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	directional3.Intensity = 1.0f;

	Light point1 = {};
	point1.Type = LIGHT_TYPE_POINT;
	point1.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	point1.Intensity = 1.0f;
	point1.Position = XMFLOAT3(-1.5f, 0, 0);
	point1.Range = 10.0f;

	Light point2 = {};
	point2.Type = LIGHT_TYPE_POINT;
	point2.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	point2.Intensity = 0.5f;
	point2.Position = XMFLOAT3(1.5f, 0, 0);
	point2.Range = 10.0f;

	lights.push_back(directional1);
	lights.push_back(directional2);
	lights.push_back(directional3);
	lights.push_back(point1);
	lights.push_back(point2);

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
// Creates the geometry we're going to draw
// TODO: maybe refactor this to be in the same method
//		 as the shaders / materials, so that I could use
//		 named local variables instead of having to
//       remember indices in the vectors?
// --------------------------------------------------------
void Game::CreateGeometry()
{
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

	// --- Create a bunch of entities ---
	// Create two entities using each mesh
	// First set uses shiny material, second uses matte
	
	// Cubes
	entities.push_back(std::make_shared<GameEntity>(
		meshes[0], materials[0]));
	// Cylinders
	entities.push_back(std::make_shared<GameEntity>(
		meshes[1], materials[0]));
	// Helixes
	entities.push_back(std::make_shared<GameEntity>(
		meshes[2], materials[0]));
	// Spheres
	entities.push_back(std::make_shared<GameEntity>(
		meshes[3], materials[0]));
	// Toruses... Tori?
	entities.push_back(std::make_shared<GameEntity>(
		meshes[4], materials[0]));
	// 1-sided Quad
	entities.push_back(std::make_shared<GameEntity>(
		meshes[5], materials[0]));
	// 2-sided Quad
	entities.push_back(std::make_shared<GameEntity>(
		meshes[6], materials[0]));
	// Second set of entities
	entities.push_back(std::make_shared<GameEntity>(
		meshes[0], materials[1]));
	entities.push_back(std::make_shared<GameEntity>(
		meshes[1], materials[1]));
	entities.push_back(std::make_shared<GameEntity>(
		meshes[2], materials[1]));
	entities.push_back(std::make_shared<GameEntity>(
		meshes[3], materials[1]));
	entities.push_back(std::make_shared<GameEntity>(
		meshes[4], materials[1]));
	entities.push_back(std::make_shared<GameEntity>(
		meshes[5], materials[1]));
	entities.push_back(std::make_shared<GameEntity>(
		meshes[6], materials[1]));

	// Move those entities, copying the positions from
	// the demo code to more easily verify my lights work
	entities[0]->GetTransform()->MoveAbsolute(-9, 0, 0);
	entities[1]->GetTransform()->MoveAbsolute(-6, 0, 0);
	entities[2]->GetTransform()->MoveAbsolute(-3, 0, 0);
	entities[3]->GetTransform()->MoveAbsolute(0, 0, 0);
	entities[4]->GetTransform()->MoveAbsolute(3, 0, 0);
	entities[5]->GetTransform()->MoveAbsolute(6, 0, 0);
	entities[6]->GetTransform()->MoveAbsolute(9, 0, 0);
	entities[7]->GetTransform()->MoveAbsolute(-9, 3, 0);
	entities[8]->GetTransform()->MoveAbsolute(-6, 3, 0);
	entities[9]->GetTransform()->MoveAbsolute(-3, 3, 0);
	entities[10]->GetTransform()->MoveAbsolute(0, 3, 0);
	entities[11]->GetTransform()->MoveAbsolute(3, 3, 0);
	entities[12]->GetTransform()->MoveAbsolute(6, 3, 0);
	entities[13]->GetTransform()->MoveAbsolute(9, 3, 0);
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
	for (unsigned int i = 0; i < entities.size(); ++i)
	{
		entities[i]->GetTransform()->Rotate(0, deltaTime, 0);
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
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(), bgColor.get());
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// DRAW entities
	for (int i = 0; i < entities.size(); ++i)
	{
		// Set a time value (if there is one)
		std::shared_ptr<SimplePixelShader> ps = entities[i]->GetMaterial()->GetPixelShader();
		ps->SetFloat("time", totalTime);
		ps->SetFloat3("ambientColor", ambientColor);
		ps->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());

		entities[i]->Draw(activeCam);
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
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
		ImGui::ColorEdit3("Ambient Light", &ambientColor.x);

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

	// Finish creating the ImGui Debug Window
	ImGui::End();
}
