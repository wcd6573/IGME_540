/*
William Duprey
10/28/24
Shader Include File
*/

// Each .hlsli file needs a unique identifier
#ifndef __GGP_SHADER_INCLUDES__ 
#define __GGP_SHADER_INCLUDES__

////////////////////////////////////////////////////////////////////////////////
// --------------------------------- STRUCTS -------------------------------- //
////////////////////////////////////////////////////////////////////////////////
// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
    // Data type
    //  |
    //  |   Name          Semantic
    //  |    |                |
    //  v    v                v
    float3 localPosition : POSITION; // XYZ position
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

// Struct representing the data we're sending down the pipeline
// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
    // Data type
    //  |
    //  |   Name          Semantic
    //  |    |                |
    //  v    v                v
    float4 screenPosition : SV_POSITION; // XYZW position (System Value Position)
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 worldPosition : POSITION;
};


////////////////////////////////////////////////////////////////////////////////
// --------------------------- HELPER FUNCTIONS ----------------------------- //
////////////////////////////////////////////////////////////////////////////////
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
inline float Voronoi(VertexToPixel input, float angle, float density)
{
    // Tile the space based on the cell density
    float3 g = floor(input.worldPosition * density);
    float3 f = frac(input.worldPosition * density);

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
                // using an angle offset
                float3 offset = random_float3(g + neighbor, angle);
                
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

#endif