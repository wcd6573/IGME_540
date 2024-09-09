// William Duprey
// 9/8/24
// Mesh Class Implementation

#include "Mesh.h"

Mesh::Mesh(Vertex* vertices, int vertexCount, int* indices, int indexCount)
{
}

Mesh::~Mesh()
{
}

Mesh::Mesh(const Mesh&)
{
}

Mesh& Mesh::operator=(const Mesh&)
{
	// TODO: insert return statement here
}

Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetVertexBuffer()
{
	return Microsoft::WRL::ComPtr<ID3D11Buffer>();
}

int Mesh::GetIndexCount()
{
	return 0;
}

Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetIndexBuffer()
{
	return Microsoft::WRL::ComPtr<ID3D11Buffer>();
}

int Mesh::GetVertexCount()
{
	return 0;
}

void Mesh::Draw(float deltaTime, float totalTime)
{
}
