/*
William Duprey
10/28/24
Shader Lighting Header
*/

#ifndef __GGP_LIGHTING__ 
#define __GGP_LIGHTING__

////////////////////////////////////////////////////////////////////////////////
// -------------------------------- CONSTANTS ------------------------------- //
////////////////////////////////////////////////////////////////////////////////
#define MAX_SPECULAR_EXPONENT 256.0f
#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

////////////////////////////////////////////////////////////////////////////////
// --------------------------------- STRUCTS -------------------------------- //
////////////////////////////////////////////////////////////////////////////////
// --------------------------------------------------------
// Custom struct that can represent directional, point, or
// spot lights. Data oriented / padded to be in line with
// the 16-byte boundary.
// --------------------------------------------------------
struct Light
{
    int Type; // Which kind of light
    float3 Direction; // Directional / Spot need direction
    float Range; // Point / Spot need max range for attenuation
    float3 Position; // Point / Spot need position in space
    float Intensity; // All lights need intensity
    float3 Color; // All lights need color
    float SpotInnerAngle; // Inner cone angle -- full light inside
    float SpotOuterAngle; // Outer cone angle -- no light outside
    float2 Padding; // Pad to hit the 16-byte boundary
};

////////////////////////////////////////////////////////////////////////////////
// -------------------------- LIGHT CALCULATIONS ---------------------------- //
////////////////////////////////////////////////////////////////////////////////
float3 calculateDiffuse(float3 normal, float3 toLight)
{
    return saturate(dot(normal, toLight));
}

float3 calculateSpecular(float3 normal, float3 toLight,
    float3 toCam, float roughness)
{
    // Calculate perfect reflection vector
    float3 reflVec = reflect(-toLight, normal);
    float specExp = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;

    
    // Return final calculated light
    return pow(saturate(dot(toCam, reflVec)), specExp);
}

// Attenuation function with non-linear falloff, and
// returns 0 when outside the light's range
float attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
}

////////////////////////////////////////////////////////////////////////////////
// ----------------------------- LIGHT TYPES -------------------------------- //
////////////////////////////////////////////////////////////////////////////////
float3 directionalLight(Light light, float3 color, float3 normal, 
    float3 cameraPos, float3 worldPos, float roughness, float specScale)
{
    // --- Diffuse ---
    float3 toLight = normalize(-light.Direction);
    float3 diffuse = calculateDiffuse(normal, toLight);
    
    // --- Specular ---
    // Calculate vector from camera to pixel
    float3 toCam = normalize(cameraPos - worldPos);
    float3 specular = calculateSpecular(normal, toLight, toCam, roughness)
        * specScale; // Scale by specular map value
    
    // Comments copied verbatim from assignment document:
    // Cut the specular if the diffuse contribution is zero
    // - any() returns 1 if any component of the param is non-zero
    // - In other words:
    //     - If the diffuse amount is 0, any(diffuse) returns 0
    //     - If the diffuse amount is != 0, any(diffuse) returns 1
    //     - So when diffuse is 0, specular becomes 0
    specular *= any(diffuse);
    
    // Combine light and return
    // Multiple specular by color? No right answer
    return ((diffuse * color) + specular) * 
        light.Intensity * light.Color;
}

float3 pointLight(Light light, float3 color, float3 normal,
    float3 cameraPos, float3 worldPos, float roughness, float specScale)
{
    // --- Diffuse ---
    // Point lights have no direction, so calculate one
    // based on the world position and light's position
    float3 toLight = normalize(light.Position - worldPos);
    float3 diffuse = calculateDiffuse(normal, toLight);
    
    // --- Specular ---
    float3 toCam = normalize(cameraPos - worldPos);
    float3 specular = calculateSpecular(normal, toLight, toCam, roughness)
        * specScale;    // Scale by specular map value

    // Combine light, scale by attenuation, and return
    return ((diffuse * color) + specular) *
        light.Intensity * light.Color * attenuate(light, worldPos);
}

#endif