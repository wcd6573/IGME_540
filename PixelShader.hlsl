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

    // --- Ambient ---
    float3 ambientLight = ambientColor * colorTint;
    
    // --- Diffuse ---
    float3 diffuseLight = calculateDiffuse(
        colorTint, input.normal, directionalLight1);
    
    // --- Specular ---
    // Calculate values that are the same for each light
    float3 viewVec = normalize(cameraPosition - input.worldPosition);
    float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
    
    // Call helper function for each Light
    float3 specularLight = calculateSpecular(input.normal, viewVec,
        specExponent, colorTint, directionalLight1);
    
    // Sum up all the light, and return it
    return float4(ambientLight + diffuseLight + specularLight, 1);
}