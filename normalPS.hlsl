/*
William Duprey
10/17/24
Normal Pixel Shader
*/

// Buffer to pass data to this pixel shader,
// only needs a color tint
// Also, each type of shader has its own registers,
// so there's no problem using b0 here too
cbuffer ExternalData : register(b0)
{
    float4 colorTint;
}

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

// --------------------------------------------------------
// Returns a color using the normal vector, the object 
// will have a gradient with red, green, and blue
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	
    return float4(input.normal, 1);
}