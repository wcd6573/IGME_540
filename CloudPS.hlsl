/*
William Duprey
12/12/24
Cloud Pixel Shader

I was inspired by this video about AAA cloud rendering:
https://www.youtube.com/watch?v=Qj_tK_mdRcA

And the accompanying repository (which uses GLSL)
https://github.com/simondevyoutube/Shaders_Clouds1/blob/main/shaders/common.glsl

Also led me to reading through this paper about how 
the Frostbite engine renders skies (section 5 on clouds):
https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/s2016-pbs-frostbite-sky-clouds-new.pdf

I also referenced this video on ray marching:
https://www.youtube.com/watch?v=BNZtUB7yhX4

I got the cube signed distance function from here:
https://iquilezles.org/articles/distfunctions/
*/

#include "ShaderIncludes.hlsli"
#include "Lighting.hlsli"

#define NUM_LIGHTS 6

cbuffer ExternalData : register(b0)
{
    float3 cameraPosition;
    
    Light lights[NUM_LIGHTS];
    int lightCount;
    
    float absorption;
    float3 objectPosition;
}

// --------------------------------------------------------
// Signed Distance Function from here:
// https://iquilezles.org/articles/distfunctions/
// --------------------------------------------------------
float sdfCube(float3 p, float3 b)
{
    float3 q = abs(p) - b;
    return length(max(q, 0.0f)) + min(max(q.x, max(q.y, q.z)), 0.0f);
}

// --------------------------------------------------------
// Renders the pixel as part of a cloud.
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{   
    // --- Specular Color Determination ---
    float3 specularColor = 0.04f;
    
    // --- Calculate Light ---
    float3 totalLight;
    
    // Calculate vector to camera, as it
    // does not change between lights
    float3 toCam = normalize(cameraPosition - input.worldPosition);
    float3 toLight; // Changes based on light type
    
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
            float3(1, 1, 1),
            specularColor,
            0,
            0
        );

        totalLight += lightResult;
    }
    
    // Calculate the distance that a ray of light 
    // would travel through the cube by ray marching
    float step = 0.1f;
    float hitThreshold = 0.1f;
    int iterations = 0;
    int maxIterations = 100;
    float3 ray = cameraPosition;
    float3 firstHit = float3(0, 0, 0);
    float3 secondHit = float3(0, 0, 0);
    bool hit = false;
    while (iterations < maxIterations)
    {
        ray += (-toCam) * step;
        bool calc = sdfCube(ray, objectPosition);
        if (!hit && calc)
        {
            firstHit = ray;
            hit = true;
        }
        else if (calc)
        {
            secondHit = ray;
            break;
        }
        
        iterations++;
    }
    
    float dist = distance(firstHit, secondHit);
    return dist;
    
    // Apply Beer's law for absorption of light
    totalLight *= exp(absorption * dist);
    
    // Perform gamma correction and return the color
    return float4(totalLight, 1);
}