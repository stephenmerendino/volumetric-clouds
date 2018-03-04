<shader>
    <shader_program>
        <vertex src="Data/HLSL/mvp.vert" />
        <fragment src="Data/HLSL/weights.frag" />
    </shader_program>

    <depth write="true" test="less" />
    <raster cull="back" fill="solid" />
    <blend>
        <color src="one" dest="zero" op="add" />
        <alpha src="one" dest="zero" op="add" />
    </blend>
</shader>
