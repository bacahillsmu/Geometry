#include "dot3_lighting.hlsl"

struct vs_input_t 
{
   float3 position      : POSITION; 
   float4 color         : COLOR; 
   float2 uv            : TEXCOORD;
   float3 normal 		: NORMAL;
   float3 tangent		: TANGENT;
   float3 bitangent     : BITANGENT;
};


cbuffer camera_constants : register(b2)
{
   float4x4 VIEW; 
   float4x4 PROJECTION; 
   
   float3 CAMERA_POSITION;
   float padcamera;
};

cbuffer object_model_buffer : register(b3)
{
   float4x4 MODEL; 
};


Texture2D<float4> tAlbedo : register(t0); // texutre I'm using for albedo (color) information
SamplerState sAlbedo : register(s0);      // sampler I'm using for the Albedo texture

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
   float4 normal 	: NORMAL;
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
   v2f.normal = world_normal;
   v2f.worldPos = world_pos.xyz;   
   v2f.tangent = world_tangent.xyz;
   v2f.bitangent = world_bitangent.xyz;
    
   return v2f;
}



//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
	float4 normalized = normalize(input.normal);
	float4 rangedNormalized = normalized + 1;
	rangedNormalized *= 0.5;
	
   // Return the normal; 
   return rangedNormalized; 
}
