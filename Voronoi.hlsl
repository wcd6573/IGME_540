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

// Pass time in to control the angle offset of the pattern
cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float time;
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
    float3 position : POSITION;
};

// --------------------------------------------------------
// Performs a bunch of arbitrary steps 
// to produce a deterministically random float3.
// --------------------------------------------------------
inline float3 random_float3(float3 pos, float offset)
{
    // 3x3 matrix of arbitrary values
    float3x3 m = float3x3(
        15.27f, 47.63f, 99.41f,
        89.98f, 95.07f, 38.39f,
        33.83f, 51.06f, 60.77f
    );
    
    // Do some arbitrary, random steps
    pos = frac(sin(mul(pos, m)) * 46839.32f);

    // Make a new float3, scaled by angle offset
    return float3(
        sin(pos.x * offset) * 0.5f + 0.5f,
        cos(pos.y * offset) * 0.5f + 0.5f,
        sin(pos.z * offset) * 0.5f + 0.5f
    );
}

// --------------------------------------------------------
// Does some stuff that's still a little over my head in
// terms of the actual Voronoi algorithm, but the result 
// is a 3D pattern of cells whose density and angle offset
// can be easily changed.
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    // This could be passed through the constant buffer,
    // but to keep it simple, I'm just hard-coding it
    float cellDensity = 1.5f;

    // Tile the space based on the cell density
    float3 g = floor(input.position * cellDensity);
    float3 f = frac(input.position * cellDensity);

    // Track the closest distance, used to color the pixel
    float minDist = 1.0f;
    
    // Loop through each neighboring tile in 3 dimensions
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            for (int z = -1; z <= 1; z++)
            {
                // Make a float3 for the neighbor cell we're looking at
                float3 neighbor = float3(x, y, z);
                
                // Get its deterministically random position,
                // using the total time elapsed as an angle offset
                float3 offset = random_float3(g + neighbor, time);
                
                // Vector to that neighbor
                float3 diff = neighbor + offset - f;

                // Magnitude of that vector
                float dist = length(diff);
                
                // Keep the closer distance
                minDist = min(minDist, dist);
            }
        }
    }
    
    return minDist;
}