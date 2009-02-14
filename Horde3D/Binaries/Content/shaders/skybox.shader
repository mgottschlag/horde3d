[[FX]]

<Sampler id="albedoMap" />

<Context id="ATTRIBPASS">
	<Shaders vertex="VS_GENERAL" fragment="FS_ATTRIBPASS" />
</Context>

<Context id="AMBIENT">
	<Shaders vertex="VS_GENERAL" fragment="FS_AMBIENT" />
</Context>


[[VS_GENERAL]]
// =================================================================================================

#include "shaders/utilityLib/vertCommon.glsl"

uniform vec3 viewer;
varying vec3 viewVec;

void main(void)
{
	vec4 pos = calcWorldPos( gl_Vertex );
	viewVec = pos.xyz - viewer;
	
	gl_Position = gl_ModelViewProjectionMatrix * pos;
}
				

[[FS_ATTRIBPASS]]
// =================================================================================================

#include "shaders/utilityLib/fragDeferredWrite.glsl"

uniform samplerCube albedoMap;
varying vec3 viewVec;

void main( void )
{
	vec3 albedo = textureCube( albedoMap, viewVec ).rgb;
	
	// Set fragment material ID to 2, meaning skybox in this case
	setMatID( 2.0 );
	setAlbedo( albedo );
}


[[FS_AMBIENT]]
// =================================================================================================

uniform samplerCube albedoMap;
varying vec3 viewVec;

void main( void )
{
	vec3 albedo = textureCube( albedoMap, viewVec ).rgb;
	
	gl_FragColor.rgb = albedo;
}