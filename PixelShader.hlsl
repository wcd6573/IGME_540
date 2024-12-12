/*
William Duprey
11/23/24
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
    float3 colorTint;
    float3 cameraPosition;
    
    float2 uvScale;
    float2 uvOffset;
    float time;
    
    Light lights[NUM_LIGHTS];
    int lightCount;
}

// t for textures, s for samplers
Texture2D Albedo        : register(t0);
Texture2D NormalMap     : register(t1);
Texture2D RoughnessMap  : register(t2);
Texture2D MetalnessMap  : register(t3);
Texture2D ShadowMap     : register(t4);

SamplerState BasicSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);

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
    // --- Adjust UVs ---
    // Scale and offset uv coordinates by given cbuffer values
    input.uv = (input.uv * uvScale) + uvOffset;
    
    // --- Shadow Mapping ---
    // Perform the perspective divide (by W) ourselves
    input.shadowMapPos /= input.shadowMapPos.w;
    
    // Convert the normalized device coordinates to UVs for sampling
    float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
    shadowUV.y = 1 - shadowUV.y;    // Flip the Y
    
    // Grab the distances we need: light-to-pixel and closest-surface
    float distToLight = input.shadowMapPos.z;
    float shadowAmount = ShadowMap.SampleCmpLevelZero(
        ShadowSampler, shadowUV, distToLight).r;
    
    // --- Sample Albedo ---
    // Sample texture to get the proper surface color
    float3 albedoColor = Albedo.Sample(BasicSampler, input.uv).rgb;
    albedoColor = pow(albedoColor, 2.2f); // Un-gamma-correct
    albedoColor *= colorTint;  // Tint using provided value
    
    // --- Sample Normal Map ---
    // Convert normals from 0 to 1 into -1 to 1
    float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
    
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
    
    // --- Sample Roughness Map ---
    // Only a single value, so just grab the r channel
    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
    
    // --- Sample Metalness Map ---
    // Same as roughness, just one value, so just use r
    float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;
    
    // --- Specular Color Determination ---
    // Assume albedo texture is actually holding 
    // specular color where metalness = 1
    // lerp -- metal is usually 0 or 1, but might be in between because 
    // of linear texture sampling, so lerp specular color to match
    float3 specularColor = lerp(F0_NON_METAL, albedoColor.rgb, metalness);
    
    // --- Calculate Light ---
    float3 totalLight;
    
    // Calculate vector to camera, as it
    // does not change between lights
    float3 toCam = normalize(cameraPosition - input.worldPosition);
    float3 toLight;  // Changes based on light type
    
    // Loop through lights 
    for (int i = 0; i < lightCount; i++)
    {
        Light light = lights[i];
        
        switch (light.Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                // Directional light has a direction,
                // use that to get vector to light
                toLight = normalize(-light.Direction);
                break;
            
            case LIGHT_TYPE_POINT:
                // Point light has no direction,
                // calculate it using its position
                // and the pixel's world position
                toLight = normalize(light.Position - input.worldPosition);
                break;
            
            case LIGHT_TYPE_SPOT:
                // Unimplemented
                toLight = float3(0, 0, 1);
                break;
        }
        
        // Calculate light
        float3 lightResult = CalculateLightPBR(
            light.Color, 
            light.Intensity, 
            input.normal, 
            toLight, 
            toCam, 
            albedoColor, 
            specularColor, 
            roughness, 
            metalness
        );
        
        // Apply shadows to only the first light
        if (i == 0)
        {
            lightResult *= shadowAmount;
        }

        totalLight += lightResult;
    }
    
    // Perform gamma correction and return the color
    return float4(pow(totalLight, 1.0f / 2.2f), 1);
}