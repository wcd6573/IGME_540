/*
William Duprey
10/16/24
Pixel Shader
Modified from starter code provided by Prof. Chris Cascioli
*/

#include "ShaderIncludes.hlsli"
#include "Lighting.hlsli"

// Buffer to pass data to this pixel shader,
// needs roughness and color tint
// Also, each type of shader has its own registers,
// so there's no problem using b0 here too
cbuffer ExternalData : register(b0)
{
    float roughness;
    float3 colorTint;
    float3 cameraPosition;
    float3 ambientColor;
    
    Light directionalLight1;
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    // Gotta normalize those normals, since they get interpolated
    // across the face of triangles, making them not unit vectors
    input.normal = normalize(input.normal);

    return float4(ambientColor * colorTint, 1);
}