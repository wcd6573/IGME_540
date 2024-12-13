/*
William Duprey
12/13/24
Pixelize Post Process Pixel Shader
*/

cbuffer ExternalData : register(b0)
{
    int pixelizeRadius;
    int pixelWidth;
    int pixelHeight;
}

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D Pixels : register(t0);
SamplerState ClampSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
    float x = int(input.uv.x) % pixelizeRadius;
    float y = int(input.uv.y) % pixelizeRadius;
    
    x = floor(pixelizeRadius / 2.0f) - x;
    y = floor(pixelizeRadius / 2.0f) - y;
    
    x = input.uv.x + x;
    y = input.uv.y + y;
    
    x *= pixelWidth;
    y *= pixelHeight;
    
    return Pixels.Sample(ClampSampler, float2(x, y));
}