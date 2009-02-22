[[FX]]

<Sampler id="buf0">
	<StageConfig addressMode="CLAMP" />
</Sampler>

<Sampler id="buf1">
	<StageConfig addressMode="CLAMP" />
</Sampler>

<Uniform id="hdrParams">
	<!-- Description:
		a - Exposure (higher values make scene brighter)
		b - Brightpass threshold (intensity where blooming begins)
		c - BrightPass offset (smaller values produce stronger blooming) 
	-->
</Uniform>
<Uniform id="blurParams" />

<Context id="BRIGHTPASS">
	<Shaders vertex="VS_FSQUAD" fragment="FS_BRIGHTPASS" />
	<RenderConfig writeDepth="false" />
</Context>

<Context id="BLUR">
	<Shaders vertex="VS_FSQUAD" fragment="FS_BLUR" />
	<RenderConfig writeDepth="false" />
</Context>

<Context id="FINALPASS">
	<Shaders vertex="VS_FSQUAD" fragment="FS_FINALPASS" />
	<RenderConfig writeDepth="false" />
</Context>


[[VS_FSQUAD]]
// =================================================================================================

varying vec2 texCoords;
				
void main( void )
{
	texCoords = gl_MultiTexCoord0.st; 
	gl_Position = gl_ProjectionMatrix * gl_Vertex;
}


[[FS_BRIGHTPASS]]
// =================================================================================================

#include "shaders/utilityLib/fragPostProcess.glsl"

uniform sampler2D buf0;
uniform vec2 frameBufSize;
uniform vec4 hdrParams;
varying vec2 texCoords;

void main( void )
{
	vec2 texSize = frameBufSize * 4.0;
	vec2 coord2 = texCoords + vec2( 2, 2 ) / texSize;
	
	// Average using bilinear filtering
	vec4 sum = getTex2DBilinear( buf0, texCoords, texSize );
	sum += getTex2DBilinear( buf0, coord2, texSize );
	sum += getTex2DBilinear( buf0, vec2( coord2.x, texCoords.y ), texSize );
	sum += getTex2DBilinear( buf0, vec2( texCoords.x, coord2.y ), texSize );
	sum /= 4.0;
	
	// Tonemap
	//sum = 1.0 - exp2( -hdrParams.x * sum );
	
	// Extract bright values
	sum = max( sum - hdrParams.y, 0.0 );
	sum /= hdrParams.z + sum;
	
	gl_FragColor = sum;
}

	
[[FS_BLUR]]
// =================================================================================================

#include "shaders/utilityLib/fragPostProcess.glsl"

uniform sampler2D buf0;
uniform vec2 frameBufSize;
uniform vec4 blurParams;
varying vec2 texCoords;

void main( void )
{
	gl_FragColor = blurKawase( buf0, texCoords, frameBufSize, blurParams.x );
}
	

[[FS_FINALPASS]]
// =================================================================================================

uniform sampler2D buf0, buf1;
uniform vec2 frameBufSize;
uniform vec4 hdrParams;
varying vec2 texCoords;

void main( void )
{
	vec4 col0 = texture2D( buf0, texCoords );	// HDR color
	vec4 col1 = texture2D( buf1, texCoords );	// Bloom
	
	// Tonemap
	vec4 col = 1.0 - exp2( -hdrParams.x * col0 );
	
	gl_FragColor = col + col1;
}
