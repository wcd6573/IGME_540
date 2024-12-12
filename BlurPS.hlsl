/*
William Duprey
12/12/24
Blur Post Process Pixel Shader
*/

struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv		: TEXCOORD0;
};

Texture2D Pixels			: register(t0);
SamplerState ClampSampler	: register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
    float4 pixelColor = Pixels.Sample(ClampSampler, input.uv);
	
	// Process the image somehow
	
    return pixelColor;
}