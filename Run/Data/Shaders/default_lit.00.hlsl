#include "dot3_lighting.hlsl"

//--------------------------------------------------------------------------------------
// Stream Input - The road.
struct vs_input_t 
{
   float3 position      : POSITION; 
   float4 color         : COLOR; 
   float2 uv            : TEXCOORD;
   float3 normal 		: NORMAL;
   float3 tangent		: TANGENT;
   float3 bitangent     : BITANGENT;
};

//--------------------------------------------------------------------------------------
// Uniform Input - World with respect to me. Where is the road given my position.
cbuffer camera_constants : register(b2)
{
   float4x4 VIEW; 
   float4x4 PROJECTION; 
   
   float3 CAMERA_POSITION;
   float padcamera;
};

// Model matrix of me.
cbuffer object_model_buffer : register(b3)
{
   float4x4 MODEL; 
};

//--------------------------------------------------------------------------------------
// Texures & Samplers
Texture2D<float4> tAlbedo : register(t0); // texutre I'm using for albedo (color) information
Texture2D<float4> tNormal : register(t1); // texutre I'm using for normal information
//Texture2D<float4> tSpec : register(t2); // not being used
Texture2D<float4> tEmissive : register(t3); // texutre I'm using for emissive information
SamplerState sAlbedo : register(s0);      // sampler I'm using for the Albedo texture

// Math
float RangeMap( float pos, float inMin, float inMax, float outMin, float outMax ) 
{ 
   return ((pos - inMin) * (outMax - outMin) / (inMax - inMin)) + outMin; 
}

 

//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// for passing data from vertex to fragment (v-2-f)
// Fragment
struct v2f_t 
{
   float4 position 	: SV_POSITION; 
   float4 color 	: COLOR; 
   float2 uv 		: UV; 
   float3 normal 	: NORMAL;
   float3 worldPos	: WORLDPOS;
   float3 tangent		: TANGENT;
   float3 bitangent     : BITANGENT;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction(vs_input_t input)
{
   v2f_t v2f = (v2f_t)0;

   float4 local_pos = float4( input.position, 1.0f ); 
   float4 world_pos = mul( MODEL, local_pos ); // mul( MODEL, local_pos ); 
   float4 view_pos = mul( VIEW, world_pos ); 
   float4 clip_pos = mul( PROJECTION, view_pos );
   float4 world_normal = mul(MODEL, float4(input.normal, 0));
   float4 world_tangent = mul(MODEL, float4(input.tangent, 0));
   float4 world_bitangent = mul(MODEL, float4(input.bitangent, 0));

   v2f.position = clip_pos; 
   v2f.color = input.color; 
   v2f.uv = input.uv;   
   v2f.worldPos = world_pos.xyz;
   v2f.normal = world_normal.xyz;
   v2f.tangent = world_tangent.xyz;
   v2f.bitangent = world_bitangent.xyz;
    
   return v2f;
}

//--------------------------------------------------------------------------------------
// Fragment Shader
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
   // First, we sample from our texture
   float4 texColor = tAlbedo.Sample( sAlbedo, input.uv ) * input.color;
   float4 normal_color = tNormal.Sample( sAlbedo, input.uv );
   
   // transform from color space to a hemisphere (z can't be negative, so we give double precision to z)
   // shorthand - map (0, 0, 0) to (1, 1, 1) -> (-1, -1, 0) to (1, 1, 1)
   float3 surface_normal = normal_color.xyz * float3(2, 2, 1) - float3(1, 1, 0); 

   float3 vertex_tangent = normalize(input.tangent); 
   float3 vertex_bitan = normalize(input.bitangent); 
   float3 vertex_normal = normalize(input.normal); 

   // commonly referred to the TBN matrix
   float3x3 surface_to_world = float3x3( vertex_tangent, vertex_bitan, vertex_normal ); 

   // if you just go with my matrix format...
   float3 world_normal = mul( surface_normal, surface_to_world ); 
   
   //GAMMA correction
   texColor = pow(texColor, GAMMA);

   lighting_t lighting = GetLighting( CAMERA_POSITION, input.worldPos, world_normal );

   //TO-DO: Add specularity!
   float4 final_color = float4(lighting.diffuse, 1.0f) * texColor;
   final_color += float4(lighting.specular, 0.f);
   final_color = pow( final_color, 1.0f / GAMMA ); // convert back to sRGB space   
   
   // component wise multiply to "tint" the output
   float4 finalColor = final_color * input.color;
   
   // EMISSIVE (map defaults to "black"); 
   float4 emissive = tEmissive.Sample( sAlbedo, input.uv ) * EMISSIVE_FACTOR; 
   finalColor += float4(emissive.xyz * emissive.w, 0);
   
   // output it; 
   //return float4(1.f, 0.f, 0.f, 1.f);
   return finalColor; 
}
