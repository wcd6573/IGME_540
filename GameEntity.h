/*
William Duprey
9/26/24
GameEntity Header
*/

#pragma once
#include <memory>

#include "Transform.h"
#include "Mesh.h"

// --------------------------------------------------------
// A class representing an entity in a game. 
// Contains a Transform, and a shared pointer to a Mesh. 
// Capable of drawing itself.
// --------------------------------------------------------
class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> _mesh);

	Transform& GetTransform();
	std::shared_ptr<Mesh> GetMesh();
	void Draw();

private:
	Transform transform;
	std::shared_ptr<Mesh> mesh;
};

