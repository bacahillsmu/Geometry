//--------------------------------------------------------------------------------------
// Stream Input
struct vs_input_t 
{
   float3 position      : POSITION; 
   float4 color         : COLOR; 
   float2 uv            : TEXCOORD; 
}; 

//--------------------------------------------------------------------------------------
// Uniform Input
cbuffer camera_constants : register(b2)
{
   float4x4 VIEW; 
   float4x4 PROJECTION; 
};

cbuffer object_model_buffer : register(b3)
{
   float4x4 MODEL; 
};

//--------------------------------------------------------------------------------------
// Texures & Samplers
Texture2D<float4> tAlbedo : register(t0); // texutre I'm using for albedo (color) information
SamplerState sAlbedo : register(s0);      // sampler I'm using for the Albedo texture

//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// for passing data from vertex to fragment (v-2-f)
struct v2f_t 
{
   float4 position : SV_POSITION; 
   float4 color : COLOR; 
   float2 uv : UV; 
}; 

//--------------------------------------------------------------------------------------
float RangeMap( float pos, float inMin, float inMax, float outMin, float outMax ) 
{ 
   return ((pos - inMin) * (outMax - outMin) / (inMax - inMin)) + outMin; 
}

//--------------------------------------------------------------------------------------
// Vertex Shader
v2f_t VertexFunction(vs_input_t input)
{
   v2f_t v2f = (v2f_t)0;

   float4 local_pos = float4( input.position, 1.0f ); 
   float4 world_pos = mul( MODEL, local_pos ); // mul( MODEL, local_pos ); 
   float4 view_pos = mul( VIEW, world_pos ); 
   float4 clip_pos = mul( PROJECTION, view_pos ); 
   
   clip_pos.z -= 1.0f;

   v2f.position = clip_pos; 
   v2f.color = input.color; 
   v2f.uv = input.uv; 
    
   return v2f;
}

//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction( v2f_t input ) : SV_Target0
{
   // First, we sample from our texture
   float4 texColor = tAlbedo.Sample( sAlbedo, input.uv ); 

   // component wise multiply to "tint" the output
   float4 finalColor = texColor * input.color;
   
   clip(finalColor.a - 0.5f);

   // output it; 
   return finalColor; 
}
