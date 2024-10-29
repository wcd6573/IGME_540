/*
William Duprey
10/28/24
Pixel Shader
Modified from starter code provided by Prof. Chris Cascioli
*/

#include "ShaderIncludes.hlsli"
#include "Lighting.hlsli"

#define NUM_LIGHTS 6

// Buffer to pass data to this pixel shader,
// needs roughness, color tint, camera position,
// and ambient light color, all for lighting calculations
cbuffer ExternalData : register(b0)
{
    float roughness;
    float3 colorTint;
    float3 cameraPosition;
    float3 ambientColor;
    float time;
    
    Light lights[NUM_LIGHTS];
}

// t for textures, s for samplers
Texture2D SurfaceTexture  : register(t0);
Texture2D SpecularMap     : register(t1);

SamplerState BasicSampler : register(s0);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    // Gotta normalize those normals, since they get interpolated
    // across the face of triangles, making them not unit vectors
    input.normal = normalize(input.normal);

    // --- Sample Textures ---
    // Sample texture to get the proper surface color
    float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb;
    surfaceColor *= colorTint;  // Tint using provided value
    
    // Sample specular map to get the scale value
    float specScale = SpecularMap.Sample(BasicSampler, input.uv).r;
    
    // --- Calculate Light ---
    // Ambient is universally applied once
    float3 totalLight = ambientColor * surfaceColor;
    
    // Loop through lights
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        Light light = lights[i];
        light.Direction = normalize(light.Direction);
        
        switch (light.Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                totalLight += directionalLight(light, surfaceColor, input.normal,
                    cameraPosition, input.worldPosition, roughness, specScale);
                break;
            case LIGHT_TYPE_POINT:
                totalLight += pointLight(light, surfaceColor, input.normal,
                    cameraPosition, input.worldPosition, roughness, specScale);
                break;
            case LIGHT_TYPE_SPOT:
                // Unimplemented
                break;
        }
    }
    
    // Sum up all the light, and return it
    return float4(totalLight, 1);
}