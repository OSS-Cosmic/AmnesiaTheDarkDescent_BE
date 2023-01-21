$input v_texcoord0

#include <common.sh>

SAMPLER2D(s_diffuseMap, 0);

uniform vec4 u_params;
#define u_alpha (u_params.x)

void main()
{
    vec4 color = texture2D(s_diffuseMap, v_texcoord0.xy);
 
    gl_FragColor.xyz = color.xyz;
    gl_FragColor.a *= u_alpha;
}