

$input v_texcoord0, v_normal, v_tangent, v_bitangent, v_position

#include <common.sh>

SAMPLERCUBE(s_envMap, 0);

SAMPLER2D(s_diffuseMap, 1);
SAMPLER2D(s_normalMap, 2);
SAMPLER2D(s_refractionMap, 3);
SAMPLER2D(s_reflectionMap, 4);

uniform mat4 u_mtxInvViewRotation;
uniform vec4 u_fogColor;

uniform vec4 u_params[4];
#define u_frenselBiasPow (u_params[0].xy)
#define u_fogStart (u_params[0].z)
#define u_fogLength (u_params[0].w)

#define u_reflectionMapSizeMul (u_params[1].xy)
#define u_reflectionFadeStart (u_params[1].z)
#define u_reflectionFadeLength (u_params[1].w)

#define u_falloffExp (u_params[2].x)
#define u_afT (u_params[2].y)
#define u_afWaveAmplitude (u_params[2].z)
#define u_afWaveFreq (u_params[2].w)

#define u_afRefractionScale (u_params[3].x)
#define u_useRefractionFading (u_params[3].y)


void main()
{
	vec2 ndc = gl_FragCoord.xy * u_viewTexel.xy;

	///////////////////////////////
	//Get the two uv coords
	float fT1 = u_afT*0.8;
	vec2 vUv1 = v_texcoord0.xy + u_afT*0.01f;
	vUv1.x += sin(fT1 + vUv1.y * u_afWaveFreq)* u_afWaveAmplitude;
	vUv1.y += sin(fT1 + vUv1.x * u_afWaveFreq)* u_afWaveAmplitude;
	
	float fT2 = u_afT*-2.6;
	vec2 vUv2 = v_texcoord0.xy + u_afT*-0.012f;
	vUv2.x += sin(fT2 + vUv2.y * u_afWaveFreq*1.2) * u_afWaveAmplitude*0.75;
	vUv2.y += sin(fT2 + vUv2.x * u_afWaveFreq*1.2) * u_afWaveAmplitude*0.75;
	
	///////////////////////////////
	//Get the normals and combine into final normal
	// (No need for full unpack since there is a normalize later)
	vec3 vNormal1 = texture2D(s_normalMap, vUv1).xyz-0.5;
	vec3 vNormal2 = texture2D(s_normalMap, vUv2).xyz-0.5;
	
	vec3 vFinalNormal = normalize(vNormal1*0.7 + vNormal2*0.3);
		
	///////////////////////////////
	//Get the diffuse color
	vec4 surfaceColor = texture2D(s_diffuseMap, vUv1);
	
	vec4 vRefractionColor = vec4(1);
	vec2 vDistortedScreenPos = vec2(0);
	#ifdef USE_REFRACTION
		float fInvDist = min(1.0 / v_position.z, 10.0);
		vDistortedScreenPos = ndc.xy + vFinalNormal.xy * u_afRefractionScale * fInvDist;
		vRefractionColor = texture2D(s_refractionMap, vDistortedScreenPos);
		if(vRefractionColor.a < 0.5) {
			vRefractionColor = texture2D(s_refractionMap, ndc.xy);
		}
	#endif
	
	
	vec4 vReflectionColor = vec4(0);
	float fFresnel = 1.0;
	#ifdef USE_REFLECTION
		//////////////////
		//Fresnel
		vec3 vScreenNormal = normalize(vFinalNormal.x * v_tangent + vFinalNormal.y * v_bitangent + vFinalNormal.z * v_normal);
		vec3 vEyeVec = normalize(v_position);
		
		float afEDotN = max(dot(-vEyeVec, vScreenNormal),0.0);
		fFresnel = Fresnel(afEDotN, u_frenselBiasPow.x, u_frenselBiasPow.y);
		
		if(0.0 < u_useRefractionFading) {
			fFresnel *= 1.0 - clamp( (v_position.z - u_reflectionFadeStart) / u_reflectionFadeLength, 0.0, 1.0);
		}

		//////////////////
		//Cubemap
		#ifdef USE_CUBE_MAP_REFLECTION
			vec3 vEnvUv = reflect(vEyeVec, vScreenNormal);
			vEnvUv = (u_mtxInvViewRotation * vec4(vEnvUv,1)).xyz;
			
			vReflectionColor = textureCube(s_envMap,vEnvUv);
		#else
			vReflectionColor = texture2D(s_reflectionMap, vDistortedScreenPos * u_reflectionMapSizeMul);
		#endif
	#endif
	

	vec3 vLightDir = normalize(vec3(0.5, 0.5, 0.5));
	float fLDotN = max(dot(vLightDir, vFinalNormal),0.0);
	float fDiffuse =  fLDotN * 0.5 + 0.5;
	float fSpecular = pow(fLDotN,16.0);

	#ifdef USE_FOG
		float fFogAmount = (-v_position.z - u_fogStart) / u_fogLength;
		fFogAmount = clamp(fFogAmount, 0.0, 1.0);
		fFogAmount = pow(fFogAmount, u_falloffExp) * u_fogColor.a;
	#else
		float fFogAmount = 0.0;
	#endif

	#ifdef USE_REFLECTION
		gl_FragColor.xyz = (surfaceColor.xyz * vRefractionColor.xyz + vReflectionColor.xyz * fFresnel) * (1.0-fFogAmount) + u_fogColor.xyz*fFogAmount;
		gl_FragColor.w = 1.0;
	#else
		gl_FragColor.xyz = (surfaceColor.xyz * vRefractionColor.xyz * fDiffuse + vec3(fSpecular)) * (1.0-fFogAmount) + u_fogColor.xyz*fFogAmount;
		gl_FragColor.w = 1.0;
	#endif

	// ///////////////////////////////
	// //Caclulate the final color
	// if(0.0 < u_useFog) {
	// 	float fFogAmount = (-v_position.z - u_fogStart) / u_fogLength;
	// 	fFogAmount = clamp(fFogAmount, 0.0, 1.0);
	// 	fFogAmount = pow(fFogAmount, u_falloffExp) * u_fogColor.a;

	// 	if(0.0 < u_useRefraction) {
	// 		if(0.0 < u_useReflection) {
	// 			gl_FragColor.xyz = (surfaceColor.xyz * vRefractionColor.xyz + vReflectionColor.xyz * fFresnel) * (1.0-fFogAmount) + u_fogColor.xyz*fFogAmount;
	// 			gl_FragColor.w = 1.0;
	// 		} else {
	// 			gl_FragColor.xyz = (surfaceColor.xyz * vRefractionColor.xyz * fDiffuse + vec3(fSpecular)) * (1.0-fFogAmount) + u_fogColor.xyz*fFogAmount;
	// 			gl_FragColor.w = 1.0;
	// 		}
	// 	} else {
	// 		gl_FragColor.xyz = (surfaceColor.xyz * vRefractionColor.xyz *fDiffuse + vec3(fSpecular)) * (1.0-fFogAmount) + vec3(fFogAmount);
	// 		gl_FragColor.w = 1.0;
	// 	}
	// } else {
	// 	if(0.0 < u_useReflection) {
	// 		gl_FragColor.xyz = vRefractionColor.xyz*surfaceColor.xyz + vReflectionColor.xyz * fFresnel;
	// 		gl_FragColor.w = 1.0;
	// 	} else {
	// 		gl_FragColor.xyz = surfaceColor.xyz * vRefractionColor.xyz *fDiffuse + vec3(fSpecular);
	// 		gl_FragColor.w = 1.0;
	// 	}
	// }
}