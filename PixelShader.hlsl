/*
William Duprey
10/29/24
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
    
    float2 uvScale;
    float2 uvOffset;
    float time;
    
    Light lights[NUM_LIGHTS];
}

// t for textures, s for samplers
Texture2D SurfaceTexture  : register(t0);
Texture2D SpecularMap     : register(t1);
Texture2D NormalMap       : register(t2);

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
    // --- Mess with input ---
    
    // Get extremely weird with voronoi, scale the
    // UVs by voronoi, angled with time, and with 3 cell density
    // Since it's just UVs, there's an ugly seam along the
    // 3D objects, but that's just how UVs work
    //input.uv *= Voronoi2D(input.uv, time, 3);
    
    // Scale and offset uv coordinates by given cbuffer values
    input.uv = (input.uv * uvScale) + uvOffset;
    
    // --- Sample Textures ---
    // Sample texture to get the proper surface color
    float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb;
    surfaceColor *= colorTint;  // Tint using provided value
    
    // Sample specular map to get the scale value
    float specScale = SpecularMap.Sample(BasicSampler, input.uv).r;
    
    // TODO: Get a blank specular map texture to give
    // to materials that do not have one so as not to
    // be dumb like this. Would it just be a white image?
    specScale = 1;
    
    // Sample normal map, converting from 0 - 1 into -1 - 1
    float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
    
    // --- Normal Map Calculations ---
    // Gotta normalize those normals, since they get interpolated
    // across the face of triangles, making them not unit vectors
    unpackedNormal = normalize(unpackedNormal);
    float3 n = normalize(input.normal);
    float3 t = normalize(input.tangent);
    
    // Gram-Schmidt for orthonormalize (ensure 90 degrees)
    t = normalize(t - n * dot(t, n));
    float3 b = cross(t, n);     // Calculate bitangent
    
    // Create rotation matrix and apply it to the normal
    float3x3 tbn = float3x3(t, b, n);
    input.normal = mul(unpackedNormal, tbn);
    
    
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