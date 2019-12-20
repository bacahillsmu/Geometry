
//--------------------------------------------------------------------------------------
// Vertex Input - currently using a built in 
// vertex index
struct vs_input_t 
{
   uint vertexIndex : SV_VERTEXID; // SV_* stands for System Variable (ie, built-in by D3D11 and has special meaning)
                                   // in this case, is the the index of the vertex coming in.
}; 

//--------------------------------------------------------------------------------------
// Uniform Input
// ------
// Uniform Data is also externally provided data, but instead of changing
// per vertex call, it is constant for all vertices, hence the name "Constant Buffer"
// or "Uniform Buffer".  This is read-only memory; 
//
// I tend to use all cap naming here, as it is effectively a 
// constant from the shader's perspective. 
//
// register(b2) determines the buffer unit/slot to use.  In this case
// we'll say this data is coming from buffer slot 2. 
//
// Last - the layout needs to match the CPU side memory layout, with some 
// caveats.
// Objects MUST start on either a 4-byte, 8-byte, or 16-byte boundary (or multiple of 16).
// Members must be on their own byte alignment, or the next larges (so float2 is on 8-byte alignment
// float3 is on 16 (since there is no 12-byte alignment)
//--------------------------------------------------------------------------------------
cbuffer camera_constants : register(b2)
{
   //float4x4 PROJECTION; 
   float2 ORTHO_MIN; 
   float2 ORTHO_MAX; 
};

//--------------------------------------------------------------------------------------
// static constants
//--------------------------------------------------------------------------------------
// The term 'static' refers to this an built into the shader, and not coming
// from a contsant buffer 
static float3 SOME_POSITIONS[3] = {
   float3(0.0f, 0.0f, 0.0f), 
   float3(20.0f, 0.0f, 0.0f),
   float3(0.0f,  20.0f, 0.0f) 
};


//--------------------------------------------------------------------------------------
// Programmable Shader Stages
//--------------------------------------------------------------------------------------

// Make the types we need to use in this shader file.
struct VertextToFragment
{
	float4 position : SV_POSITION; // We always need this one. We use float4 because it is a vector.
};

// You can declare other functions, similar to C++
float RangeMap( float pos, float inMin, float inMax, float outMin, float outMax ) 
{
   return ((pos - inMin) * (outMax - outMin) / (inMax - inMin)) + outMin; 
}

//--------------------------------------------------------------------------------------
// Vertex Shader
VertextToFragment VertexFunction(vs_input_t input)
{
   VertextToFragment v2f = (VertextToFragment)0;
   float3 inPos = SOME_POSITIONS[input.vertexIndex]; 

   // we'll be getting a float3, but have to output a float4
   // which is the output in "clip space". 
   //
   // Only outputs in the following ranges will display..
   // [-1, 1] on the x (left to right)
   // [-1, 1] on the y (bottom to top)
   // [ 0, 1] on the z 

   // (note: technically clip_space is a homogeneous coordinate
   //  so the above is not 100% accurate, but more on that later)

   float4 localPos = float4( inPos, 1.0f ); 
   float4 clipPos = float4(0.0f, 0.0f, 0.0f, 1.0f);
   clipPos.x = RangeMap(localPos.x, ORTHO_MIN.x, ORTHO_MAX.x, -1, 1);
   clipPos.y = RangeMap(localPos.y, ORTHO_MIN.y, ORTHO_MAX.y, -1, 1);

   // For now, we'll just set w to 1, and forward.
   v2f.position = clipPos; 

   return v2f;
}

//--------------------------------------------------------------------------------------
// Fragment Shader
// 
// SV_Target0 at the end means the float4 being returned
// is being drawn to the first bound color target.
float4 FragmentFunction( VertextToFragment input ) : SV_Target0
{
   // outputting yellow; 
   return float4(1.0f, 1.0f, 0.0f, 1.0f ); 
}
