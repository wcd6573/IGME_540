/*
William Duprey
10/17/24
3D Voronoi Noise Shader

This is based on part of a shadergraph I made in Unity
over the summer for a co-op, which in turn was based
on these resources:

Referenced this shader code made for Unity from 
https://github.com/JimmyCushnie/Noisy-Nodes/tree/master,
Voronoi3D contributed by @fdervaux.

Also read through this explanation (which uses GLSL) 
to better understand Voronoi: https://thebookofshaders.com/12/ 
by Patricio Gonzales Vivo and Jen Lowe
*/

#include "ShaderIncludes.hlsli"

// Pass time in to control the angle offset of the pattern
cbuffer ExternalData : register(b0)
{
    float3 colorTint;
    float time;
}

// --------------------------------------------------------
// Simply calls the Voronoi helper function.
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    return Voronoi(input, time, 2.5f);
}