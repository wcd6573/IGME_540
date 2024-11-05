/*
William Duprey
11/5/24
Sky Vertex Shader
*/

// Sky vertex shader only needs view and projection,
// no need for world / worldInverseTranspose
cbuffer ExternalData : register(b0)
{
    matrix view;
    matrix projection;
}

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}