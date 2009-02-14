[[FX]]

<Sampler id="heightNormMap" />
<Sampler id="detailMap" />

<Uniform id="sunDir" a="1.0" b="-1.0" c="0.0" />

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

#include "shaders/utilityLib/vertCommon.glsl"

uniform vec4 terBlockParams;
attribute float terHeight;
varying vec4 pos, vsPos;
varying vec2 texCoords;

void main( void )
{
	vec4 newPos = vec4( gl_Vertex.x * terBlockParams.z + terBlockParams.x, terHeight,
						gl_Vertex.z * terBlockParams.z + terBlockParams.y, gl_Vertex.w );
						
	pos = calcWorldPos( newPos );
	vsPos = calcViewPos( pos );
						
	texCoords = vec2( newPos.x, newPos.z );
	gl_Position = gl_ModelViewProjectionMatrix * pos;
}


[[FS_ATTRIBPASS]]
// =================================================================================================

#include "shaders/utilityLib/fragDeferredWrite.glsl"

uniform vec4 sunDir;
uniform sampler2D heightNormMap, detailMap;
varying vec4 pos;
varying vec2 texCoords;

vec3 light = -normalize( sunDir.xyz );

void main( void )
{
	vec3 detailMap = texture2D( detailMap, texCoords * 300.0 ).rgb;
	vec4 texel = texture2D( heightNormMap, texCoords ) * 2.0 - 1.0;
	float ny = sqrt( max( 1.0 - texel.b*texel.b - texel.a*texel.a, 0.0 ) );		// Use max because of numerical issues
	vec3 normal = vec3( texel.b, ny, texel.a );
	
	setMatID( 1.0 );
	setPos( pos.xyz );
	setNormal( normalize( normal ) );
	setAlbedo( detailMap );
	setSpecMask( 0.0 );
}
				

[[VS_SHADOWMAP]]
// =================================================================================================

#include "shaders/utilityLib/vertCommon.glsl"

uniform vec4 lightPos;
uniform vec4 terBlockParams;
attribute float terHeight;
varying float dist;

void main( void )
{
	vec4 newPos = vec4( gl_Vertex.x * terBlockParams.z + terBlockParams.x, terHeight,
						gl_Vertex.z * terBlockParams.z + terBlockParams.y, gl_Vertex.w );
						
	vec4 pos = calcWorldPos( newPos );
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
}


[[FS_LIGHTING]]
// =================================================================================================

#include "shaders/utilityLib/fragLighting.glsl"

uniform sampler2D heightNormMap, detailMap;
varying vec4 pos, vsPos;
varying vec2 texCoords;

void main( void )
{
	vec3 detailMap = texture2D( detailMap, texCoords * 300.0 ).rgb;
	vec4 texel = texture2D( heightNormMap, texCoords ) * 2.0 - 1.0;
	float ny = sqrt( max( 1.0 - texel.b*texel.b - texel.a*texel.a, 0.0 ) );		// Use max because of numerical issues
	vec3 normal = vec3( texel.b, ny, texel.a );
	
	gl_FragColor.rgb =
		calcPhongSpotLight( pos.xyz, normalize( normal ), detailMap, 0.0, 16.0, -vsPos.z, 0.3 );
}


[[FS_AMBIENT]]
// =================================================================================================

uniform vec4 sunDir;
uniform sampler2D heightNormMap, detailMap;
varying vec2 texCoords;

vec3 light = -normalize( sunDir.xyz );

void main( void )
{
	vec3 detailMap = texture2D( detailMap, texCoords * 300.0 ).rgb;
	vec4 texel = texture2D( heightNormMap, texCoords ) * 2.0 - 1.0;
	float ny = sqrt( max( 1.0 - texel.b*texel.b - texel.a*texel.a, 0.0 ) );		// Use max because of numerical issues
	vec3 normal = vec3( texel.b, ny, texel.a );
	
	// Wrap lighting fur sun
	float l = max( dot( normal, light ), 0.0 ) * 0.5 + 0.5;
	
	gl_FragColor = vec4( detailMap * l, 1.0 );
}