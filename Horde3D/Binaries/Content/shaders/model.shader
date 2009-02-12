[[FX]]
<!--
// =================================================================================================
	Model Shader
	
	Supported Flags:
		_F01_Skinning
		_F02_NormalMapping
		_F03_ParallaxMapping
		_F04_EnvMapping
// =================================================================================================
-->

<Sampler id="albedoMap" />
<Sampler id="normalMap" />
<Sampler id="ambientMap" />
<Sampler id="envMap" />

<Uniform id="specParams" a="0.1" b="16.0">
	<!-- Description
		a - Specular mask
		b - Specular exponent
	-->
</Uniform>


<Context id="ATTRIBPASS">
	<Shaders vertex="VS_GENERAL" fragment="FS_ATTRIBPASS" />
</Context>

<Context id="SHADOWMAP">
	<Shaders vertex="VS_SHADOWMAP" fragment="FS_SHADOWMAP" />
</Context>

<Context id="LIGHTING">
	<Shaders vertex="VS_GENERAL" fragment="FS_LIGHTING" />
	<RenderConfig writeDepth="false" blendMode="ADD" />
</Context>

<Context id="AMBIENT">
	<Shaders vertex="VS_GENERAL" fragment="FS_AMBIENT" />
</Context>


[[VS_GENERAL]]
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "utilityLib/vertCommon.glsl"

#ifdef _F01_Skinning
	#include "utilityLib/vertSkinning.glsl"
#endif

uniform vec3 viewer;
attribute vec2 texCoords0;
attribute vec3 normal;

#ifdef _F02_NormalMapping
	attribute vec3 tangent, bitangent;
#endif

varying vec4 pos, vsPos;
varying vec2 texCoords;

#ifdef _F02_NormalMapping
	varying mat3 tsbMat;
#else
	varying vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	varying vec3 eyeTS;
#endif


void main( void )
{
#ifdef _F01_Skinning
	mat4 skinningMat = calcSkinningMat();
	mat3 skinningMatVec = getSkinningMatVec( skinningMat );
#endif
	
	// Calculate normal
#ifdef _F01_Skinning
	vec3 _normal = calcWorldVec( skinVec( normal, skinningMatVec ) );
#else
	vec3 _normal = calcWorldVec( normal );
#endif

	// Calculate tangent and bitangent
#ifdef _F02_NormalMapping
	#ifdef _F01_Skinning
		vec3 _tangent = calcWorldVec( skinVec( tangent, skinningMatVec ) );
		vec3 _bitangent = calcWorldVec( skinVec( bitangent, skinningMatVec ) );
	#else
		vec3 _tangent = calcWorldVec( tangent );
		vec3 _bitangent = calcWorldVec( bitangent );
	#endif
	
	tsbMat = calcTanToWorldMat( _tangent, _bitangent, _normal );
#else
	tsbNormal = _normal;
#endif

	// Calculate world space position
#ifdef _F01_Skinning	
	pos = calcWorldPos( skinPos( gl_Vertex, skinningMat ) );
#else
	pos = calcWorldPos( gl_Vertex );
#endif

	vsPos = calcViewPos( pos );

	// Calculate tangent space eye vector
#ifdef _F03_ParallaxMapping
	eyeTS = calcTanVec( viewer - pos.xyz, _tangent, _bitangent, _normal );
#endif
	
	// Calculate texture coordinates and clip space position
	texCoords = texCoords0;
	gl_Position = gl_ModelViewProjectionMatrix * pos;
}


[[FS_ATTRIBPASS]]
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "utilityLib/fragDeferredWrite.glsl" />

uniform vec4 specParams;
uniform sampler2D albedoMap;

#ifdef _F02_NormalMapping
	uniform sampler2D normalMap;
#endif

varying vec4 pos;
varying vec2 texCoords;

#ifdef _F02_NormalMapping
	varying mat3 tsbMat;
#else
	varying vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	varying vec3 eyeTS;
#endif

void main( void )
{
	vec3 newCoords = vec3( texCoords, 0 );
	
#ifdef _F03_ParallaxMapping	
	const float plxScale = 0.03;
	const float plxBias = -0.015;
	
	// Iterative parallax mapping
	vec3 eye = normalize( eyeTS );
	for( int i = 0; i < 4; ++i )
	{
		vec4 nmap = texture2D( normalMap, newCoords.st );
		float height = nmap.a * plxScale + plxBias;
		newCoords += (height - newCoords.p) * nmap.z * eye;
	}
#endif

	vec3 albedo = texture2D( albedoMap, newCoords.st ).rgb;
	
#ifdef _F02_NormalMapping
	vec3 normalMap = texture2D( normalMap, newCoords.st ).rgb * 2.0 - 1.0;
	vec3 normal = tsbMat * normalMap;
#else
	vec3 normal = tsbNormal;
#endif

	vec3 newPos = pos.xyz;

#ifdef _F03_ParallaxMapping
	newPos += vec3( 0.0, newCoords.p, 0.0 );
#endif
	
	setMatID( 1.0 );
	setPos( newPos );
	setNormal( normalize( normal ) );
	setAlbedo( albedo );
	setSpecMask( specParams.x );
}

	
[[VS_SHADOWMAP]]
// =================================================================================================
	
#include "utilityLib/vertCommon.glsl"
#include "utilityLib/vertSkinning.glsl"

uniform vec4 lightPos;
varying float dist;

void main( void )
{
	vec4 pos = calcWorldPos( skinPos( gl_Vertex ) );
	dist = length( lightPos.xyz - pos.xyz ) / lightPos.w;
	
	gl_Position = gl_ModelViewProjectionMatrix * pos;
}
	
	
[[FS_SHADOWMAP]]
// =================================================================================================

uniform float shadowBias;
varying float dist;

void main( void )
{
	gl_FragDepth = dist + shadowBias;
	
	// Clearly better bias but requires SM 3.0
	// gl_FragDepth =  dist + abs( dFdx( dist ) ) + abs( dFdy( dist ) ) + shadowBias;
}


[[FS_LIGHTING]]
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "utilityLib/fragLighting.glsl" />

uniform vec4 specParams;
uniform sampler2D albedoMap;

#ifdef _F02_NormalMapping
	uniform sampler2D normalMap;
#endif

varying vec4 pos, vsPos;
varying vec2 texCoords;

#ifdef _F02_NormalMapping
	varying mat3 tsbMat;
#else
	varying vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	varying vec3 eyeTS;
#endif

void main( void )
{
	vec3 newCoords = vec3( texCoords, 0 );
	
#ifdef _F03_ParallaxMapping	
	const float plxScale = 0.03;
	const float plxBias = -0.015;
	
	// Iterative parallax mapping
	vec3 eye = normalize( eyeTS );
	for( int i = 0; i < 4; ++i )
	{
		vec4 nmap = texture2D( normalMap, newCoords.st );
		float height = nmap.a * plxScale + plxBias;
		newCoords += (height - newCoords.p) * nmap.z * eye;
	}
#endif

	vec3 albedo = texture2D( albedoMap, newCoords.st ).rgb;
	
#ifdef _F02_NormalMapping
	vec3 normalMap = texture2D( normalMap, newCoords.st ).rgb * 2.0 - 1.0;
	vec3 normal = tsbMat * normalMap;
#else
	vec3 normal = tsbNormal;
#endif

	vec3 newPos = pos.xyz;

#ifdef _F03_ParallaxMapping
	newPos += vec3( 0.0, newCoords.p, 0.0 );
#endif
	
	gl_FragColor.rgb =
		calcPhongSpotLight( newPos, normalize( normal ), albedo, specParams.x, specParams.y, -vsPos.z, 0.3 );
}


[[FS_AMBIENT]]	
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "utilityLib/fragLighting.glsl" />

uniform sampler2D albedoMap;
uniform samplerCube ambientMap;

#ifdef _F02_NormalMapping
	uniform sampler2D normalMap;
#endif

#ifdef _F04_EnvMapping
	uniform samplerCube envMap;
#endif

varying vec4 pos;
varying vec2 texCoords;

#ifdef _F02_NormalMapping
	varying mat3 tsbMat;
#else
	varying vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	varying vec3 eyeTS;
#endif

void main( void )
{
	vec3 newCoords = vec3( texCoords, 0 );
	
#ifdef _F03_ParallaxMapping	
	const float plxScale = 0.03;
	const float plxBias = -0.015;
	
	// Iterative parallax mapping
	vec3 eye = normalize( eyeTS );
	for( int i = 0; i < 4; ++i )
	{
		vec4 nmap = texture2D( normalMap, newCoords.st );
		float height = nmap.a * plxScale + plxBias;
		newCoords += (height - newCoords.p) * nmap.z * eye;
	}
#endif

	vec3 albedo = texture2D( albedoMap, newCoords.st ).rgb;
	
#ifdef _F02_NormalMapping
	vec3 normalMap = texture2D( normalMap, newCoords.st ).rgb * 2.0 - 1.0;
	vec3 normal = tsbMat * normalMap;
#else
	vec3 normal = tsbNormal;
#endif
	
	gl_FragColor.rgb = albedo * textureCube( ambientMap, normal ).rgb;
	
#ifdef _F04_EnvMapping
	vec3 refl = textureCube( envMap, reflect( pos.xyz - viewer, normalize( normal ) ) ).rgb;
	gl_FragColor.rgb = refl * 1.5;
#endif
}