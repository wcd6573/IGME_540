// William Duprey
// 9/8/24
// Mesh Class Header

#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Graphics.h"
#include "Vertex.h"


// --------------------------------------------------------
// A class that stores data on a 3D mesh. Rather than be 
// a "dumb" container, it can draw itself.
// --------------------------------------------------------
class Mesh
{
public:
	// Underscores used for potentially ambiguous param names
	Mesh(Vertex* vertices, UINT _vertexCount, 
		UINT* indices, UINT _indexCount,
		const char* _name);
	~Mesh();

	// Copy constructor and copy assignment operator (scary)
	Mesh(const Mesh& other);
	Mesh& operator=(const Mesh& other);

	// Getters for the private fields
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	UINT GetIndexCount();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	UINT GetVertexCount();
	const char* GetName();

	// Sets buffers and draws the mesh to the screen
	void Draw(float deltaTime, float totalTime);

private:
	// Vertices of the triangles making up the mesh
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	UINT vertexCount;

	// Indices of the vertices of the triangles making up the mesh
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	UINT indexCount;

	// Name of the mesh for ImGui to display
	const char* name;
};

