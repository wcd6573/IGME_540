// William Duprey
// 9/5/24
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
	CreateGeometry();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

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

	// Set up extra float array for coloring text
	textColor = std::make_shared<float[]>(4);
	for (int i = 0; i < 4; i++)
	{
		textColor[i] = 1.0f;
	}
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

	// Set up starter triangle vertices and indices
	Vertex vertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int indices[] = { 0, 1, 2 };

	// Add the default starter code triangle
	//meshes.push_back(std::make_shared<Mesh>(
	//	vertices, 3, indices, 3));
	
	Vertex hatVertices[] =
	{
		// Comment coordinates relative to x: 0-12, y: 0-14
		{ XMFLOAT3(+0.0f, 0.25f, +0.0f), black },	// 0, 3
		{ XMFLOAT3(0.07f, +.33f, +0.0f), grey },	// 1, 4
		{ XMFLOAT3(0.07f, +.17f, +0.0f), black },	// 1, 2
		{ XMFLOAT3(0.14f, 0.25f, +0.0f), grey },	// 2, 3
		{ XMFLOAT3(0.21f, 0.08f, +0.0f), black },	// 3, 1
		{ XMFLOAT3(.286f, +.17f, +0.0f), grey },	// 4, 2
		{ XMFLOAT3(.429f, +0.0f, +0.0f), black },	// 6, 0
		{ XMFLOAT3(.714f, +.17f, +0.0f), grey },	// 10, 2
		{ XMFLOAT3(.571f, +0.0f, +0.0f), black },	// 8, 0
		{ XMFLOAT3(.786f, +.08f, +0.0f), black },	// 11, 1
		{ XMFLOAT3(.857f, +.25f, +0.0f), grey },	// 12, 3
	};

	unsigned int hatIndices[] = { 
		0, 1, 2,
		1, 2, 3,
		2, 3, 4,
		3, 4, 5,
		4, 5, 6,
		5, 6, 7,
		6, 7, 8,
		7, 8, 9,
		8, 10, 11,
	};

	meshes.push_back(std::make_shared<Mesh>(
		hatVertices, 11, hatIndices, 27));
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
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	bgColor.get());
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// DRAW geometry
	for (int i = 0; i < meshes.size(); ++i) 
	{
		meshes[i]->Draw(deltaTime, totalTime);
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

	// Create a collapsable header for App Details
	if (ImGui::CollapsingHeader("App Details"))
	{
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
	}

	// Extra (pointless) UI for Assignment 2 requirement
	BuildExtraUI();

	// Finish creating the ImGui Debug Window
	ImGui::End();
}


// --------------------------------------------------------
// Creates some more UI elements that don't really do
// anything. Just practicing and messing around with ImGui.
// 
// I may have been a bit lax with coding standards here
// just because I'll probably delete this method before 
// the next assignment.
// 
// Broken off into this method to make it that much
// easier to remove later.
// --------------------------------------------------------
void Game::BuildExtraUI()
{
	// Collapsable header for extra test things
	if (ImGui::CollapsingHeader("Testing ImGui Elements\n(to be deleted before next assignment)"))
	{
		// Messing around with ImGui table 
		// This makes a 2x2 grid of float sliders for a color
		// R	G
		// B	A
		// Each PushStyleColor call needs its own ImVec4, so this
		// whole process is probably not worth it just to have some colored text
		// 
		// Also, read through this stackoverflow thread to 
		// figure out how to dynamically change text color
		// https://stackoverflow.com/questions/61853584/how-can-i-change-text-color-of-my-inputtext-in-imgui
		ImGui::Text("An alternative (worse) color selector!");
		
		// This label doesn't seem to show up anywhere?
		ImGui::BeginTable("Table Test", 2);	

		// Start at row 0 col 0
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);

		// Red slider
		ImGui::PushStyleColor(ImGuiCol_Text,
			ImVec4(textColor[0], 0, 0, 1));
		ImGui::SliderFloat("R", &textColor[0], 0, 1);
		ImGui::PopStyleColor();

		// Go to row 0 col 1
		ImGui::TableNextColumn();

		// Green slider
		ImGui::PushStyleColor(ImGuiCol_Text,
			ImVec4(0, textColor[1], 0, 1));
		ImGui::SliderFloat("G", &textColor[1], 0, 1);
		ImGui::PopStyleColor();

		// Go to row 1 col 0
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);

		// Blue slider
		ImGui::PushStyleColor(ImGuiCol_Text,
			ImVec4(0, 0, textColor[2], 1));
		ImGui::SliderFloat("B", &textColor[2], 0, 1);
		ImGui::PopStyleColor();

		// Go to row 1 col 1
		ImGui::TableNextColumn();

		// Alpha slider
		ImGui::PushStyleColor(ImGuiCol_Text,
			ImVec4(1, 1, 1, textColor[3]));
		ImGui::SliderFloat("A", &textColor[3], 0, 1);
		ImGui::PopStyleColor();
		ImGui::EndTable();

		// If checked, color the text next to the checkbox 
		// with the color values from the above table
		if (updateTextColor)
		{
			ImGui::PushStyleColor(ImGuiCol_Text,
				ImVec4(
					textColor[0], textColor[1],
					textColor[2], textColor[3]));
			ImGui::Checkbox("Color me surprised", &updateTextColor);
			ImGui::PopStyleColor();
		}
		else
		{
			ImGui::Checkbox("Color me", &updateTextColor);
		}

		// Dropdown 
		//		into a dropdown 
		//			into a dropdown 
		//				into a button that closes all of them
		if(ImGui::CollapsingHeader("Dropdown"))
		{
			if(ImGui::CollapsingHeader("to a dropdown"))
			{
				if (ImGui::CollapsingHeader("to yet another dropdown"))
				{
					if (ImGui::Button("to rock button"))
					{
						// Found this forum post to figure out how to
						// programmatically close a dropdown
						// https://www.unknowncheats.me/forum/programming-for-beginners/387081-imgui-collapsingheader.html
						ImGui::GetStateStorage()->SetInt(
							ImGui::GetID("Dropdown"), 0);
						ImGui::GetStateStorage()->SetInt(
							ImGui::GetID("to a dropdown"), 0);
						ImGui::GetStateStorage()->SetInt(
							ImGui::GetID("to yet another dropdown"), 0);
					}
				}
			}
		}
	}
}

