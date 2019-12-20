<shader id="default/unlit_transparent">

   <!-- A "pass" is our current Shader.  It is all the state of rendering pipeline.
        Later on, we will add support for multiple pass shaders, so having this syntax to be forward compatible -->
   <pass src="Data/Shaders/sprite.hlsl">
      <vert entry="VertexFunction" />
      <frag entry="FragmentFunction" />

      <!-- depth will use "always" and write="false" by default to maintain 2D functionality -->
      <!-- That is effectively saying depth is disabled -->
      <!-- for 3D, you want to set "write" to true and test to "lequal" -->
      <!-- test:  test: never, less, equal, lequal, greater, gequal, not, always -->
      <depth
         write="true"
         test="lequal" />


      <!-- I'm open to options here.  You can expose "op" and "factors", or support a "mode", or both.  But at least
           have some way to control blending for color -->

      <!-- Example Shown is using op/factors.  You may choose just an enum.  IE: <blend mode="transparent" /> -->
      <!-- op: add, sub, rev_sub, min, max -->
      <!-- src/dst: zero, one, src_color, inv_src_color, src_alpha, inv_src_alpha, 
                    dst_color, inv_dst_color, dst_alpha, inv_dst_alpha, constant -->
      <blend>
         <!-- setting this to "opaque".  We usually don't render alpha if we're writing depth -->
         <!-- <color op="add" src="one" dst="zero" /> -->
         <!-- <alpha op="add" src="inv_dst_alpha" dst="one" />  -->
		 
		 <!-- <color mode="alpha" /> -->
		 <!-- <color mode="opaque" />-->
         <color mode="opaque" />
		 
      </blend>

      <!-- OPTIONAL EXTRA :: RasterState -->
      <!-- cull: back, front, none -->
      <!-- windCCW: true, false = (ccw, cw) -->
      <!-- fill: solid, wire -->
      <raster
         cull="back"       
         windCCW="true"
         fill="solid" />
   </pass>
</shader>
