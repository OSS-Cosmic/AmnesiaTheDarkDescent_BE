$input a_texcoord0, a_texcoord1, a_position
$output v_texcoord0, v_texcoord1

#include <bgfx_shader.sh>

void main()
{
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
    v_texcoord0 = a_texcoord0;
    v_texcoord1 = a_texcoord1;
}