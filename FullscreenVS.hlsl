/*
William Duprey
12/12/24
Fullscreen Vertex Shader
*/

struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv		: TEXCOORD0;
};

// --------------------------------------------------------
// SV_VertexID means the pipeline will provide a unique id 
// for each vertex, which is used to determine pos and UVs.
// --------------------------------------------------------
VertexToPixel main( uint id : SV_VertexID )
{
    VertexToPixel output;
    
    // Branchless code because branches are evil
    output.uv = float2(
        (id << 1) & 2,  // id % 2 * 2
        id & 2);

    // Calculate position based on UV
    output.position = float4(output.uv, 0, 1);
    output.position.x = output.position.x * 2 - 1;
    output.position.y = output.position.y * -2 + 1;
    
    return output;
}