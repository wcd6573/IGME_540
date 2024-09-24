/*
William Duprey
9/24/24
BufferStructs Header
*/

#pragma once
#include <DirectXMath.h>

/*
A struct used for transferring data to the GPU.
The vertex shader has a cbuffer that matches this struct.
*/
struct VertexShaderExternalData
{
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT4X4 world;
};

