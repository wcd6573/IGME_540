// William Duprey
// 9/24/24
// Game Class Implementation
// Modified from starter code provided by Prof. Chris Cascioli

#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "BufferStructs.h"

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
	CreateGeometry();

	// Initialize the constant buffer for the vertex shader
	{
		// Set up the constant buffer description
		D3D11_BUFFER_DESC cbDesc = {};
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		// Size must be a multiple of 16, use math to ensure that
		cbDesc.ByteWidth = (sizeof(VertexShaderExternalData) + 15) / 16 * 16;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;

		// Actually create the buffer, sets the ComPtr
		Graphics::Device->CreateBuffer(
			&cbDesc, 0, vsConstBuffer.GetAddressOf());

		// Bind the buffer to the pipeline
		Graphics::Context->VSSetConstantBuffers(
			0, 1, vsConstBuffer.GetAddressOf());
	}

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
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
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
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
		{ XMFLOAT3(-.214f, -.33f, +0.0f), darkGrey },// 5 -- (4, 2)
		{ XMFLOAT3(-.071f, -0.5f, +0.0f), grey },	// 6 -- (6, 0)
		{ XMFLOAT3(.214f, -.33f, +0.0f), darkGrey },// 7 -- (10, 2)
		{ XMFLOAT3(.071f, -0.5f, +0.0f), grey },	// 8 -- (8, 0)
		{ XMFLOAT3(.286f, -.42f, +0.0f), grey },	// 9 -- (11, 1)
		{ XMFLOAT3(.357f, -.25f, +0.0f), grey },	// 10 - (12, 3)
		{ XMFLOAT3(.429f, -.33f, +0.0f), grey },	// 11 - (13, 2)
		{ XMFLOAT3(.429f, -.17f, +0.0f), grey },	// 12 - (13, 4)
		{ XMFLOAT3(+0.5f, -.25f, +0.0f), grey },	// 13 - (14, 3)
		// Vertices for the band:							
		{ XMFLOAT3(.286f, -.083, +0.0f), darkGrey },// 14 - (11, 5)
		{ XMFLOAT3(-.286f, -.083f, +0.0f), darkGrey },// 15 - (3, 5)
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
	std::shared_ptr<GameEntity> hat1 = std::make_shared<GameEntity>(meshes[1]);
	std::shared_ptr<GameEntity> hat2 = std::make_shared<GameEntity>(meshes[1]);
	std::shared_ptr<GameEntity> hat3 = std::make_shared<GameEntity>(meshes[1]);
	std::shared_ptr<GameEntity> quad1 = std::make_shared<GameEntity>(meshes[2]);
	std::shared_ptr<GameEntity> tri1 = std::make_shared<GameEntity>(meshes[0]);

	// Alter positions so that they're not all on top of each other


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
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Update ImGui at the start of the frame so it has fresh data
	NewFrameUI(deltaTime);
	BuildUI();

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
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
		entities[i].get()->Draw(vsConstBuffer);
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
		}
		ImGui::TreePop();
	}

	// Create a collapsible header for controller the Vertex shader
	if (ImGui::TreeNode("Vertex Shader External Data"))
	{
		ImGui::SliderFloat3("Offset", offset.get(), -1, 1);
		ImGui::ColorEdit4("Tint", colorTint.get());
		ImGui::TreePop();
	}

	// Finish creating the ImGui Debug Window
	ImGui::End();
}

