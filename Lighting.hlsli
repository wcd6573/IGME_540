/*
William Duprey
10/24/24
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


#endif