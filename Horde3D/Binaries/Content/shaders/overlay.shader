[[FX]]

<Context id="OVERLAY">
	<Shaders vertex="VS_OVERLAY" fragment="FS_OVERLAY" />
	<RenderConfig writeDepth="false" blendMode="BLEND" />
</Context>

[[VS_OVERLAY]]

varying vec2 texCoord;

void main( void )
{
	texCoord = gl_MultiTexCoord0.st; 
	gl_Position = gl_ProjectionMatrix * gl_Vertex;
}


[[FS_OVERLAY]]

uniform vec4 olayColor;
uniform sampler2D tex0;
varying vec2 texCoord;

void main( void )
{
	vec4 albedo = texture2D( tex0, texCoord );
	
	gl_FragColor = albedo * olayColor;
}