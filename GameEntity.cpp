/*
William Duprey
10/28/24
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
	transform = std::make_shared<Transform>();
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
std::shared_ptr<Transform> GameEntity::GetTransform()
{
	return transform;
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
	// Set up shaders and shader data
	material->PrepareMaterial(transform, camera);

	// Set vertex / index buffers and draw using the mesh
	mesh->SetBuffersAndDraw();
}
