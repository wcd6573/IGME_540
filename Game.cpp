// William Duprey
// 10/3/24
// Game Class Implementation
// Modified from starter code provided by Prof. Chris Cascioli

#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"

#include <DirectXMath.h>

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
	LoadShaders();
	CreateMaterials();
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

	// Set some initial values for the constant buffer struct
	colorTint = std::make_shared<float[]>(4);
	colorTint[0] = 0.5f;
	colorTint[1] = 0.5f;
	colorTint[2] = 1.0f;
	colorTint[3] = 1.0f;
	offset = std::make_shared<float[]>(3);
	offset[0] = -0.25f;
	offset[1] = -0.10f;
	offset[2] = 0.0f;

	// --- Create a bunch of cameras ---
	// Standard, default camera
	cameras.push_back(std::make_shared<Camera>(
		XMFLOAT3(0, 0, -5),
		Window::AspectRatio()
	));
	
	// Unmoving (unless changed with ImGui), orthographic camera
	cameras.push_back(std::make_shared<Camera>(
		XMFLOAT3(3, 1, -3),
		Window::AspectRatio(),
		XM_PIDIV4,
		0.01f,
		100.0f,
		false,			// Don't do perspective (do orthographic)
		7.5f,			// Ortho width
		0.0f,			// No move speed
		0.0f			// No look speed
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
// Uses SimpleShader to set up necessary shaders.
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, 
		FixPath(L"VertexShader.cso").c_str());
	pixelShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context,
		FixPath(L"PixelShader.cso").c_str());
}

// --------------------------------------------------------
// Sets up some simple materials,
// before any entities are made.
// --------------------------------------------------------
void Game::CreateMaterials()
{
	// --- Create some Materials ---
	// Purple color tint
	materials.push_back(std::make_shared<Material>(
		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),
		vertexShader,
		pixelShader
	));

	// Grey color tint
	materials.push_back(std::make_shared<Material>(
		XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),
		vertexShader,
		pixelShader
	));

	// Teal color tint
	materials.push_back(std::make_shared<Material>(
		XMFLOAT4(0.0f, 0.7f, 0.7f, 1.0f),
		vertexShader,
		pixelShader
	));
}

// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1, 1, 1, 1);
	XMFLOAT4 black = XMFLOAT4(0, 0, 0, 1);
	XMFLOAT4 darkGrey = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	XMFLOAT4 grey = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself

	// -------------------- TRIANGLE ----------------------
	// Set up starter triangle vertices and indices
	Vertex triVertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.25f, +0.0f), red },
		{ XMFLOAT3(+.25f, -0.25f, +0.0f), blue },
		{ XMFLOAT3(-0.25f, -0.25f, +0.0f), green },
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int triIndices[] = { 0, 1, 2 };

	// Add the default starter code triangle
	meshes.push_back(std::make_shared<Mesh>(
		triVertices, ARRAYSIZE(triVertices),
		triIndices, ARRAYSIZE(triIndices),
		"Starter Triangle"));

	// --------------------- TOP HAT ----------------------
	// Vertices to create a top hat! It took a lot of fiddling
	// with the index buffer before I really understood what I
	// was doing, but I'm pretty satisfied with the result.
	Vertex hatVertices[] =
	{
		// Comment coordinates relative to x: 0-12, y: 0-14
		// (I sketched this out on graph paper and just labeled points
		// like this as I went. Would this have been much simpler if
		// I just used x: 0-10 and y: 0-10? Yes.)
		// Vertices for the brim:
		{ XMFLOAT3(-0.5f, -.25f, +0.0f), grey },	// 0 -- (0, 3)
		{ XMFLOAT3(-.43f, -.17f, +0.0f), grey },	// 1 -- (1, 4)
		{ XMFLOAT3(-.43f, -.33f, +0.0f), grey },	// 2 -- (1, 2)
		{ XMFLOAT3(-.36f, -.25f, +0.0f), grey },	// 3 -- (2, 3)
		{ XMFLOAT3(-.29f, -.42f, +0.0f), grey },	// 4 -- (3, 1)
		{ XMFLOAT3(-.214f, -.33f, +0.0f),darkGrey },// 5 -- (4, 2)
		{ XMFLOAT3(-.071f, -0.5f, +0.0f), grey },	// 6 -- (6, 0)
		{ XMFLOAT3(.214f, -.33f, +0.0f), darkGrey },// 7 -- (10, 2)
		{ XMFLOAT3(.071f, -0.5f, +0.0f), grey },	// 8 -- (8, 0)
		{ XMFLOAT3(.286f, -.42f, +0.0f), grey },	// 9 -- (11, 1)
		{ XMFLOAT3(.357f, -.25f, +0.0f), grey },	// 10 - (12, 3)
		{ XMFLOAT3(.429f, -.33f, +0.0f), grey },	// 11 - (13, 2)
		{ XMFLOAT3(.429f, -.17f, +0.0f), grey },	// 12 - (13, 4)
		{ XMFLOAT3(+0.5f, -.25f, +0.0f), grey },	// 13 - (14, 3)
		// Vertices for the band:							
		{ XMFLOAT3(.286f, -.083f, +0.0f), darkGrey },// 14 - (11, 5)
		{ XMFLOAT3(-.286f, -.083f,+0.0f), darkGrey },// 15 - (3, 5)
		// Vertices for the top:							
		{ XMFLOAT3(.429f, +0.5f, +0.0f), grey },	// 16 - (13, 12)
		{ XMFLOAT3(-.43f, +0.5f, +0.0f), grey },	// 17 - (1, 12)
	};

	// Vertices must be clockwise for triangle to face correctly
	unsigned int hatIndices[] = {
		0, 1, 2,		// Brim
		2, 1, 3,
		2, 3, 4,
		4, 3, 5,
		4, 5, 6,
		6, 5, 7,
		6, 7, 8,
		8, 7, 9,
		9, 7, 10,
		9, 10, 11,
		11, 10, 12,
		11, 12, 13,
		5, 14, 7,		// Band
		5, 15, 14,
		15, 16, 14,		// Top
		15, 17, 16
	};

	// It looks fine, but I sort of wish the edges of the band
	// were sharper. Either make a new mesh to draw over it, or 
	// maybe add some vertices in between?
	meshes.push_back(std::make_shared<Mesh>(
		hatVertices, ARRAYSIZE(hatVertices),
		hatIndices, ARRAYSIZE(hatIndices),
		"Top Hat"));

	// ---------------------- QUAD ------------------------
	// Now for something comparatively simpler: a quadrilateral
	Vertex quadVertices[] =
	{
		{XMFLOAT3(-.15f, +0.25f, 0), black},
		{XMFLOAT3(0.25f, +0.25f, 0), grey},
		{XMFLOAT3(-.25f, -0.25f, 0), white},
		{XMFLOAT3(0.15f, -0.25f, 0), blue},
	};

	unsigned int quadIndices[] =
	{
		0, 1, 2,
		1, 3, 2,
	};

	meshes.push_back(std::make_shared<Mesh>(
		quadVertices, ARRAYSIZE(quadVertices),
		quadIndices, ARRAYSIZE(quadIndices),
		"Quad"));


	// --------------- MAKE SOME ENTITIES -----------------
	// Create shared pointers using the above meshes
	std::shared_ptr<GameEntity> hat1 = std::make_shared<GameEntity>(
		meshes[1], materials[0]);
	std::shared_ptr<GameEntity> hat2 = std::make_shared<GameEntity>(
		meshes[1], materials[1]);
	std::shared_ptr<GameEntity> hat3 = std::make_shared<GameEntity>(
		meshes[1], materials[2]);
	std::shared_ptr<GameEntity> quad1 = std::make_shared<GameEntity>(
		meshes[2], materials[1]);
	std::shared_ptr<GameEntity> tri1 = std::make_shared<GameEntity>(
		meshes[0], materials[2]);

	// Alter positions so that they're not all on top of each other
	hat1.get()->GetTransform()->MoveAbsolute(0.5f, 0.5f, 0);
	
	hat2.get()->GetTransform()->MoveAbsolute(0.75f, -0.75f, 0);
	hat2.get()->GetTransform()->Scale(0.1f, 0.1f, 0);
	
	hat3.get()->GetTransform()->MoveAbsolute(-0.5f, 0, 0);
	hat3.get()->GetTransform()->Scale(0.5f, 1.5f, 0);
	hat3.get()->GetTransform()->Rotate(0, 0, XM_PI);

	quad1.get()->GetTransform()->MoveAbsolute(0.1f, -0.65f, 0);

	// Add all entities to the vector
	entities.push_back(hat1);
	entities.push_back(hat2);
	entities.push_back(hat3);
	entities.push_back(quad1);
	entities.push_back(tri1);
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
	// Scale calculation taken from Demo code
	float scale = (float)sin(totalTime * 5) * 0.5f + 1.0f;
	
	// Make this hat move like a jellyfish
	entities[2].get()->GetTransform()->SetScale(1.5f-scale, scale, 1);
	entities[2].get()->GetTransform()->
		SetPosition(-0.5f, (float)sin(totalTime) * 0.5f, 0);

	// Big hat rotates counterclockwise, small hat rotates clockwise
	entities[0].get()->GetTransform()->Rotate(0, 0, deltaTime);
	entities[1].get()->GetTransform()->Rotate(0, 0, -deltaTime);

	// Small hat orbits around (0.5f, -0.5f)
	entities[1].get()->GetTransform()->SetPosition(
		0.5f + (float)cos(totalTime) * 0.1f,
		-0.5f + (float)sin(totalTime) * 0.1f, 0);

	// Quad and Tri each just move along the x and y axes respectively
	entities[3].get()->GetTransform()->SetPosition((float)sin(totalTime), 0, 0);
	entities[4].get()->GetTransform()->SetPosition(0, (float)sin(totalTime), 0);

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
		entities[i].get()->Draw(activeCam);
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
				ImGui::Spacing();

				// Get pointer to transform and each field of it
				Transform* trans = entities[i].get()->GetTransform();
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

	// Finish creating the ImGui Debug Window
	ImGui::End();
}
