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

// --------------------------------------------------------
// A class representing an entity in a game. 
// Contains a Transform, and a shared pointer to a Mesh. 
// Capable of drawing itself.
// --------------------------------------------------------
class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> _mesh);

	// No destructor since it would be really bad if
	// an entity destroyed a Mesh it was sharing

	Transform* GetTransform();
	std::shared_ptr<Mesh> GetMesh();
	void Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstBuffer);

private:
	Transform transform;
	std::shared_ptr<Mesh> mesh;
};

