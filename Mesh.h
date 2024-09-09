// William Duprey
// 9/8/24
// Mesh Class Header

#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Graphics.h"
#include "Vertex.h"

// A class that stores data on a 3D mesh. Rather than be a "dumb"
// container, it can draw itself.
class Mesh
{
public:
	// Underscores used for potentially ambiguous param names
	Mesh(Vertex* vertices, int _vertexCount, int* indices, int _indexCount);
	~Mesh();

	// Removing these since I don't want to have to figure out how
	// to copy over a ComPtr<ID3D11Buffer>
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	// Getters for the private fields
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	int GetIndexCount();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetVertexCount();

	// Sets buffers and draws the mesh to the screen
	void Draw(float deltaTime, float totalTime);

private:
	// Vertices of the triangles making up the mesh
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	int vertexCount;

	// Indices of the vertices of the triangles making up the mesh
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	int indexCount;
};

