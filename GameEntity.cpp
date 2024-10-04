/*
William Duprey
9/26/24
GameEntity Implementation
*/

#include "GameEntity.h"
#include "BufferStructs.h"
using namespace DirectX;

// --------------------------------------------------------
// Constructor for a GameEntity. Creates a new Transform,
// and sets the given shared pointer to a Mesh.
// --------------------------------------------------------
GameEntity::GameEntity(std::shared_ptr<Mesh> _mesh)
{
	transform = Transform();
	mesh = _mesh;
}

// --------------------------------------------------------
// Returns a pointer to the entity's Transform, allowing
// it to be modified by code outside of this class.
// --------------------------------------------------------
Transform* GameEntity::GetTransform()
{
	return &transform;
}

// --------------------------------------------------------
// Simple getter for the Mesh shared pointer.
// --------------------------------------------------------
std::shared_ptr<Mesh> GameEntity::GetMesh()
{
	return mesh;
}

// --------------------------------------------------------
// Draws the GameEntity using its transform, mesh, and
// the constant buffer passed in.
// Note: this code could go in a separate "Renderer" class,
//		 if I felt like doing that way
// --------------------------------------------------------
void GameEntity::Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstBuffer,
	std::shared_ptr<Camera> camera)
{
	// Create the struct to send data to the vertex shader:
	//	- Get the world matrix from the transform, 
	//	- and just tint blue by default
	VertexShaderExternalData vsData;
	vsData.world = transform.GetWorldMatrix();
	vsData.colorTint = XMFLOAT4(0.5f, 0.5f, 1.0f, 1.0f);
	vsData.view = camera->GetViewMatrix();
	vsData.projection = camera->GetProjectionMatrix();

	// Bind constant buffer
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	Graphics::Context->Map(
		vsConstBuffer.Get(), 0,
		D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

	// Brutally efficient copy data to GPU
	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));

	// Stop using the constant buffer
	Graphics::Context->Unmap(vsConstBuffer.Get(), 0);

	// Finally, call the mesh draw method
	// (also sets vertex and index buffers)
	mesh->SetBuffersAndDraw();
}
