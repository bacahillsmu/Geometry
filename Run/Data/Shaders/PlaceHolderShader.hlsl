#include "dot3_lighting.hlsl"

//--------------------------------------------------------------------------------------
// Stream Input - The road.
struct vs_input_t 
{
   float3 position      : POSITION; 
   float4 color         : COLOR; 
   float2 uv            : TEXCOORD;
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
   float3 worldPos	: WORLDPOS;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction(vs_input_t input)
{
   v2f_t v2f = (v2f_t)0;

   float4 local_pos = float4( input.position, 1.0f ); 
   float4 world_pos = mul( MODEL, local_pos );
   float4 view_pos = mul( VIEW, world_pos ); 
   float4 clip_pos = mul( PROJECTION, view_pos );

   v2f.position = clip_pos; 
   v2f.color = input.color; 
   v2f.uv = input.uv;   
   v2f.worldPos = world_pos.xyz;
    
   return v2f;
}

//--------------------------------------------------------------------------------------
// Fragment Shader
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
   // First, we sample from our texture
   float4 texColor = tAlbedo.Sample( sAlbedo, input.uv ) * input.color;
   
   // component wise multiply to "tint" the output
   float4 finalColor = texColor * float4(0.0f, 1.0f, 0.0f, 0.5f);
   
   return finalColor; 
}
