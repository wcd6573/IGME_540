/*
William Duprey
11/5/24
Sky Vertex Shader
*/

#include "ShaderIncludes.hlsli"

// Sky vertex shader only needs view and projection,
// no need for world / worldInverseTranspose
cbuffer ExternalData : register(b0)
{
    matrix view;
    matrix projection;
}

// --------------------------------------------------------
// The entry point for the sky vertex shader. Sets up 
// the VertexToPixel_Sky struct for the sky pixel shader.
// --------------------------------------------------------
VertexToPixel_Sky main( VertexShaderInput input ) : SV_POSITION
{
    VertexToPixel_Sky output;
    
    // Get view matrix, but with no translation,
    // so that the cube stays at the origin (camera)
    matrix viewNoTranslate = view;
    viewNoTranslate._14 = 0;
    viewNoTranslate._24 = 0;
    viewNoTranslate._34 = 0;
    
    // Apply projection and view to local position
    matrix vp = mul(projection, viewNoTranslate);
    output.position = mul(vp, float4(input.localPosition, 1.0f));
    
    // Ensure output depth is exactly 1.0 after the shader,
    // after automatic perspective divide in the rasterizer
    output.position.z = output.position.w;
    
    // Sample direction is just the vertex's position, since
    // the cube and camera are centered at the origin
    output.sampleDir = input.localPosition;
    
    return output;
}