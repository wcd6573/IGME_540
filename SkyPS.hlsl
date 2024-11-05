/*
William Duprey
11/5/24
Sky Pixel Shader
*/

#include "ShaderIncludes.hlsli"

// 6 texture subresources required for the CubeMap
TextureCube CubeMap : register(t0);
SamplerState BasicSampler : register(s0);

// --------------------------------------------------------
// The entry point for the sky pixel shader. Samples the
// cube map at the provided direction.
// --------------------------------------------------------
float4 main(VertexToPixel_Sky input) : SV_TARGET
{
    return CubeMap.Sample(BasicSampler, input.sampleDir);
}