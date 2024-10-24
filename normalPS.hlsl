/*
William Duprey
10/17/24
Normal Pixel Shader
*/

#include "ShaderIncludes.hlsli"

// Buffer to pass data to this pixel shader,
// only needs a color tint
// Also, each type of shader has its own registers,
// so there's no problem using b0 here too
cbuffer ExternalData : register(b0)
{
    float3 colorTint;
}

// --------------------------------------------------------
// Returns a color using the normal vector, the object 
// will have a gradient with red, green, and blue
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	
    return float4(input.normal, 1);
}