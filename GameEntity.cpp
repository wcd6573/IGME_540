/*
William Duprey
9/26/24
GameEntity Implementation
*/

#include "GameEntity.h"
using namespace DirectX;

// --------------------------------------------------------
// Constructor for a GameEntity. Creates a new Transform,
// and sets the given shared pointer to a Mesh.
// --------------------------------------------------------
GameEntity::GameEntity(std::shared_ptr<Mesh> _mesh,
	std::shared_ptr<Material> _material)
{
	transform = Transform();
	mesh = _mesh;
	material = _material;
}

///////////////////////////////////////////////////////////////////////////////
// ------------------------------- GETTERS --------------------------------- //
///////////////////////////////////////////////////////////////////////////////
// --------------------------------------------------------
// Returns a pointer to the entity's Transform, allowing
// it to be modified by code outside of this class.
// --------------------------------------------------------
Transform* GameEntity::GetTransform()
{
	return &transform;
}

std::shared_ptr<Mesh> GameEntity::GetMesh() { return mesh; }
std::shared_ptr<Material> GameEntity::GetMaterial() { return material; }


///////////////////////////////////////////////////////////////////////////////
// ------------------------------- SETTERS --------------------------------- //
///////////////////////////////////////////////////////////////////////////////
void GameEntity::SetMesh(std::shared_ptr<Mesh> _mesh) { mesh = _mesh; }
void GameEntity::SetMaterial(std::shared_ptr<Material> _material) { material = _material; }


// --------------------------------------------------------
// Draws the GameEntity using its transform, mesh, and
// the constant buffer passed in.
// Note: this code could go in a separate "Renderer" class,
//		 if I felt like doing that way
// --------------------------------------------------------
void GameEntity::Draw(std::shared_ptr<Camera> camera)
{
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();

	// Activate the correct shaders
	vs->SetShader();
	ps->SetShader();

	// Strings must exactly match variable names in shader cbuffer
	vs->SetMatrix4x4("world", transform.GetWorldMatrix());
	vs->SetMatrix4x4("view", camera->GetViewMatrix());
	vs->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	
	// Copy data to the GPU
	vs->CopyAllBufferData();

	// Do the same for the pixel shader (only needs color tint)
	ps->SetFloat4("colorTint", material->GetColorTint());
	ps->CopyAllBufferData();

	// Finally, call the mesh draw method
	// (also sets vertex and index buffers)
	mesh->SetBuffersAndDraw();
}
