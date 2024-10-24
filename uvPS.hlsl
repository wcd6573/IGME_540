/*
William Duprey
10/17/24
UV Pixel Shader
*/

#include "ShaderIncludes.hlsli"

// Buffer to pass data to this pixel shader,
// only needs a color tint
// Also, each type of shader has its own registers,
// so there's no problem using b0 here too
cbuffer ExternalData : register(b0)
{
    float4 colorTint;
}

// --------------------------------------------------------
// Returns a color using the 2D uv coordinates,
// the object will have a gradient from red to green
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	return float4(input.uv, 0, 1);
}