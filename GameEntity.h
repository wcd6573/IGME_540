/*
William Duprey
9/26/24
GameEntity Header
*/

#pragma once
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>

#include "Mesh.h"
#include "Transform.h"
#include "Camera.h"
#include "Material.h"

// --------------------------------------------------------
// A class representing an entity in a game. 
// Contains a Transform, and a shared pointer to a Mesh. 
// Capable of drawing itself.
// --------------------------------------------------------
class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> _mesh,
		std::shared_ptr<Material> _material);

	// No destructor since it would be really bad if
	// an entity destroyed a Mesh it was sharing

	Transform* GetTransform();
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();

	void SetMesh(std::shared_ptr<Mesh> _mesh);
	void SetMaterial(std::shared_ptr<Material> _material);

	void Draw(std::shared_ptr<Camera> camera);

private:
	Transform transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
};

