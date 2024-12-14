/*
William Duprey
12/13/24
Pixelize Post Process Pixel Shader
*/

cbuffer ExternalData : register(b0)
{
    int pixelizeRadius;
    float pixelWidth;
    float pixelHeight;
}

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D Pixels : register(t0);
SamplerState ClampSampler : register(s0);

// ---------------------------------------------------------
// Pixelizing shader, adapted from GLSL code here:
// https://lettier.github.io/3d-game-shaders-for-beginners/pixelization.html
// ---------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    float x = uint(input.position.x) % pixelizeRadius;
    float y = uint(input.position.y) % pixelizeRadius;
    
    x = floor(pixelizeRadius / 2.0f) - x;
    y = floor(pixelizeRadius / 2.0f) - y;
    
    x = input.position.x + x;
    y = input.position.y + y;
    
    x *= pixelWidth;
    y *= pixelHeight;

    return Pixels.Sample(ClampSampler, float2(x, y));
}