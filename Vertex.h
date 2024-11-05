#pragma once

#include <DirectXMath.h>

// --------------------------------------------------------
// A custom vertex definition
// --------------------------------------------------------
struct Vertex
{
	DirectX::XMFLOAT3 Position;	    // The local position of the vertex
	DirectX::XMFLOAT3 Normal;		// The normal vector of the vertex
	DirectX::XMFLOAT3 Tangent;		// The tangent, perpendicular to normal
	DirectX::XMFLOAT2 UV;			// The UV coordinate of the vertex
};