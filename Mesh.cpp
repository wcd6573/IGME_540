// William Duprey
// 9/8/24
// Mesh Class Implementation

#include "Mesh.h"


///////////////////////////////////////////////////////////////////////////////
// ----------------------------- MESH CLASS -------------------------------- //
///////////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------------------
// Constructor for a Mesh object. Initializes the vertex and index buffers
// necessary to define the Mesh in terms the GPU can work with.
// 
// Code mostly copied from Game.cpp starter code (including comments, 
// so I can continue attempting to understand things).
// ----------------------------------------------------------------------------
Mesh::Mesh(Vertex* vertices, UINT _vertexCount, UINT* indices, UINT _indexCount,
	const char* _name)
{
	name = _name;
	vertexCount = _vertexCount;
	indexCount = _indexCount;

	// Create a VERTEX BUFFER
	// - This holds the vertex data of triangles for a single object
	// - This buffer is created on the GPU, which is where the data needs to
	//    be if we want the GPU to act on it (as in: draw it to the screen)
	{
		// First, we need to describe the buffer we want Direct3D to make on the GPU
		//  - Note that this variable is created on the stack since we only need it once
		//  - After the buffer is created, this description variable is unnecessary
		D3D11_BUFFER_DESC vbd = {};
		vbd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
		vbd.ByteWidth = sizeof(Vertex) * vertexCount; // number of vertices in the buffer
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Tells Direct3D this is a vertex buffer
		vbd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;

		// Create the proper struct to hold the initial vertex data
		// - This is how we initially fill the buffer with data
		// - Essentially, we're specifying a pointer to the data to copy
		D3D11_SUBRESOURCE_DATA initialVertexData = {};
		initialVertexData.pSysMem = vertices; // pSysMem = Pointer to System Memory

		// Actually create the buffer on the GPU with the initial data
		// - Once we do this, we'll NEVER CHANGE DATA IN THE BUFFER AGAIN
		Graphics::Device->CreateBuffer(&vbd, &initialVertexData, vertexBuffer.GetAddressOf());
	}

	// Create an INDEX BUFFER
	// - This holds indices to elements in the vertex buffer
	// - This is most useful when vertices are shared among neighboring triangles
	// - This buffer is created on the GPU, which is where the data needs to
	//    be if we want the GPU to act on it (as in: draw it to the screen)
	{
		// Describe the buffer, as we did above, with two major differences
		//  - Byte Width (3 unsigned integers vs. 3 whole vertices)
		//  - Bind Flag (used as an index buffer instead of a vertex buffer) 
		D3D11_BUFFER_DESC ibd = {};
		ibd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
		ibd.ByteWidth = sizeof(unsigned int) * indexCount; // number of indices in the buffer
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;	// Tells Direct3D this is an index buffer
		ibd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		// Specify the initial data for this buffer, similar to above
		D3D11_SUBRESOURCE_DATA initialIndexData = {};
		initialIndexData.pSysMem = indices; // pSysMem = Pointer to System Memory

		// Actually create the buffer with the initial data
		// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
		Graphics::Device->CreateBuffer(&ibd, &initialIndexData, indexBuffer.GetAddressOf());
	}
}


// --------------------------------------------------------
// Mesh destructor. ComPtrs will clean themselves up, 
// so there's nothing here that needs deleting (yet).
// --------------------------------------------------------
Mesh::~Mesh()
{
}

// --------------------------------------------------------
// Copy constructor for a Mesh. Hopefully no memory leaks!
// ComPtrs will ensure that no D3D objects are
// left floating around in memory (I hope).
// --------------------------------------------------------
Mesh::Mesh(const Mesh& other)
{
	// Copy over vertex buffer fields
	vertexBuffer = other.vertexBuffer;
	vertexCount = other.vertexCount;

	// Copy over index buffer fields
	indexBuffer = other.indexBuffer;
	indexCount = other.indexCount;
}

// --------------------------------------------------------
// Copy assignment operator for a Mesh.
// ComPtrs will ensure that no D3D objects are
// left floating around in memory (I hope).
// --------------------------------------------------------
Mesh& Mesh::operator=(const Mesh& other)
{
	// Don't let a Mesh copy to itself
	if (this != &other)
	{
		// Copy over vertex buffer fields
		vertexBuffer = other.vertexBuffer;
		vertexCount = other.vertexCount;

		// Copy over index buffer fields
		indexBuffer = other.indexBuffer;
		indexCount = other.indexCount;
	}

	return *this;
}


///////////////////////////////////////////////////////////////////////////////
// ------------------------------- GETTERS --------------------------------- //
///////////////////////////////////////////////////////////////////////////////
Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetVertexBuffer()
{
	return vertexBuffer;
}

UINT Mesh::GetVertexCount()
{
	return vertexCount;
}

Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetIndexBuffer()
{
	return indexBuffer;
}

UINT Mesh::GetIndexCount()
{
	return indexCount;
}

const char* Mesh::GetName()
{
	return name;
}


///////////////////////////////////////////////////////////////////////////////
// --------------------------------- DRAW ---------------------------------- //
///////////////////////////////////////////////////////////////////////////////
// --------------------------------------------------------
// Sets up vertex and index buffers for the input assembler,
// then draws the indexed vertices.
// 
// Game::Draw has these parameters, which I don't
// think are used anywhere, but I have them here too,
// in case they're ever needed.
// 
// Code mostly copied from the Game::Draw() starter code,
// including comments, so I can continue to understand.
// --------------------------------------------------------
void Mesh::Draw(float deltaTime, float totalTime)
{
	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	{
		// Set buffers in the input assembler (IA) stage
		//  - Do this ONCE PER OBJECT, since each object may have different geometry
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		Graphics::Context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		Graphics::Context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Tell Direct3D to draw
		//  - Begins the rendering pipeline on the GPU
		//  - Do this ONCE PER OBJECT you intend to draw
		//  - This will use all currently set Direct3D resources (shaders, buffers, etc)
		//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
		//     vertices in the currently set VERTEX BUFFER
		Graphics::Context->DrawIndexed(
			indexCount, // The number of indices to use (we could draw a subset if we wanted)
			0,			// Offset to the first index we want to use
			0);			// Offset to add to each index when looking up vertices
	}
}
