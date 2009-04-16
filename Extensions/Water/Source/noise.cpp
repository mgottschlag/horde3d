// *************************************************************************************************
//
// Horde3D Water Extension
// --------------------------------------------------------
// Copyright (C) 2009 Mathias Gottschlag
//
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// *************************************************************************************************

#include "noise.h"
#include "utOpenGL.h"
#include "egModules.h"

extern "C" void glfwSwapBuffers();

namespace Horde3DWater
{
	static const char *normalVP =
		"varying vec2 texcoord;\n"
		"uniform vec4 weights;\n"
		"void main() {\n"
		"	texcoord = gl_Vertex.xy;\n"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
		"}";
	static const char *normalFP =
		"varying vec2 texcoord;\n"
		"uniform sampler2D tex0;\n"
		"void main() {\n"
		"	vec3 v1 = vec3(10.0/256.0, texture2D(tex0, texcoord + vec2(1.0/1024.0, 0.0)).r\n"
		"	                         - texture2D(tex0, texcoord - vec2(1.0/1024.0, 0.0)).r, 0.0);\n"
		"	vec3 v2 = vec3(0.0, texture2D(tex0, texcoord + vec2(0.0, 1.0/1024.0)).r\n"
		"	                  - texture2D(tex0, texcoord - vec2(0.0, 1.0/1024.0)).r, 10.0/256.0);\n"
		"	vec3 normal = normalize(cross(v2, v1));\n"
		"	gl_FragColor.rgb = normal;\n"
		"}\n";

	NoiseResource::NoiseResource( const std::string &name, int flags, int octaves )
		: Resource( RT_NoiseResource, name, flags ), _octaves( octaves ),
		_falloff( 0.5 ), _time( 0.0 ), _current( 0 )
	{
		_loaded = true;
		// Create noise textures
		for( int frame = 0; frame < NOISE_FRAMES; frame++ )
		{
			glGenTextures( 1, &_noiseTex[frame] );
			glBindTexture( GL_TEXTURE_2D, _noiseTex[frame] );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, NOISE_SIZE, NOISE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
		}
		glGenTextures( MAX_OCTAVES, _octaveTex);
		for( int o = 0; o < MAX_OCTAVES; o++ )
		{
			glBindTexture( GL_TEXTURE_2D, _octaveTex[o] );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
			float *data = new float[NOISE_SIZE_SQ * 4];
			for( int i = 0; i < NOISE_SIZE_SQ; i++ )
			{
				data[i * 4] = 0;
			}
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, NOISE_SIZE, NOISE_SIZE, 0, GL_RGBA, GL_FLOAT, data );
		}
		glGenFramebuffersEXT( 1, &_FBO );
		glGenFramebuffersEXT( 1, &_FBO2 );
		// TODO: Better names
		_heightMap = new TextureResource("noise_heightmap", 0, 1024, 1024, false);
		Modules::resMan().addNonExistingResource( *_heightMap, true );
		glBindTexture( GL_TEXTURE_2D, _heightMap->getTexObject() );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, 1024, 1024, 0, GL_RGBA, GL_FLOAT, 0 );
		_normalMap = new TextureResource("noise_normalmap", 0, 1024, 1024, false);
		Modules::resMan().addNonExistingResource( *_normalMap, true );
		glBindTexture( GL_TEXTURE_2D, _normalMap->getTexObject() );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, 1024, 1024, 0, GL_RGBA, GL_FLOAT, 0 );
		_normalProgram = ((RendererBase&)Modules::renderer()).uploadShader( normalVP, normalFP );
		glLinkProgram( _normalProgram );
		if( !_normalProgram )
		{
			printf( "Water: Normal map program not available.\n" );
			std::string msg = Modules::renderer().getShaderLog();
			if (msg != "")
				printf( "Error:\n%s\n", msg.c_str() );
		}
		// Generate noise
		calcOctWeights();
		initNoise();
		calcNoise();
	}
	NoiseResource::~NoiseResource()
	{
		// TODO
	}

	void NoiseResource::initNoise()
	{
		// Create white noise
		srand( time( 0 ) );
		int srcNoise[NOISE_SIZE_SQ * NOISE_FRAMES];
		for( int i = 0; i < NOISE_SIZE_SQ * NOISE_FRAMES; i++ )
		{
			float value = (float)rand() / RAND_MAX;
			srcNoise[i] = value * SCALE;
		}
		// Generate smooth noise
		for( int frame = 0; frame < NOISE_FRAMES; frame++ )
		{
			int f = frame * NOISE_SIZE_SQ;
			for( int y = 0; y < NOISE_SIZE; y++ )
			{
				int y0 = ((y - 1) & (NOISE_SIZE - 1)) * NOISE_SIZE;
				int y1 = y * NOISE_SIZE;
				int y2 = ((y + 1) & (NOISE_SIZE - 1)) * NOISE_SIZE;
				for( int x = 0; x < NOISE_SIZE; x++ )
				{
					int x0 = (x - 1) & (NOISE_SIZE - 1);
					int x1 = x;
					int x2 = (x + 1) & (NOISE_SIZE - 1);
					int value = srcNoise[f + x0 + y0] +     srcNoise[f + x1 + y0] + srcNoise[f + x2 + y0]
					          + srcNoise[f + x0 + y1] + 6 * srcNoise[f + x1 + y1] + srcNoise[f + x2 + y1]
					          + srcNoise[f + x0 + y2] +     srcNoise[f + x1 + y2] + srcNoise[f + x2 + y2];
					_noise[f + x1 + y1] = value / 14;
				}
			}
			// Fill the noise textures
			glBindTexture( GL_TEXTURE_2D, _noiseTex[frame] );
			float *data = new float[NOISE_SIZE_SQ * 4];
			for( int i = 0; i < NOISE_SIZE_SQ; i++ )
			{
				data[i * 4] = (float)srcNoise[f + i] / SCALE;
				data[i * 4 + 1] = (float)srcNoise[f + i] / SCALE;
				data[i * 4 + 2] = (float)srcNoise[f + i] / SCALE;
				data[i * 4 + 3] = (float)srcNoise[f + i] / SCALE;
			}
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, NOISE_SIZE, NOISE_SIZE, 0, GL_RGBA, GL_FLOAT, data );
			/*char *data = new char[NOISE_SIZE_SQ * 4];
			for( int i = 0; i < NOISE_SIZE_SQ; i++ )
			{
				data[i * 4] = srcNoise[f + i];
				data[i * 4 + 1] = srcNoise[f + i];
				data[i * 4 + 2] = srcNoise[f + i];
				data[i * 4 + 3] = srcNoise[f + i];

			}
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, NOISE_SIZE, NOISE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );*/
			
			delete[] data;
		}
	}
	void NoiseResource::calcNoise()
	{
		glPushAttrib( GL_VIEWPORT_BIT );
		glDisable( GL_DEPTH_TEST );
		glDisable( GL_CULL_FACE );
		glDisable( GL_ALPHA_TEST );
		glDisable( GL_SAMPLE_ALPHA_TO_COVERAGE );
		glClearDepth( 1.0f );
		glDisable( GL_BLEND );
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glBlendFunc( GL_ONE, GL_ONE );
		glEnable( GL_BLEND );
		glUseProgram( 0 );
		for( uint32 i = 0; i < 12; ++i )
		{
			glActiveTexture( GL_TEXTURE0 + i );
			glBindTexture( GL_TEXTURE_2D, 0 );
			glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
		}
		glActiveTexture( GL_TEXTURE0 );
		glEnable( GL_TEXTURE_2D );

		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		glOrtho( 0, 1, 0, 1, 0, 1 );
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		// Interpolate between frames
		glViewport( 0, 0, NOISE_SIZE, NOISE_SIZE );
		glClearColor( 0.0, 0.0, 0.0, 1.0 );
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _FBO );
		float timefactor = 1.0f;
		for( int o = 0; o < _octaves; o++ )
		{
			double dframe;
			float fraction = modf( _time * timefactor, &dframe );
			int frame = dframe;
			// Interpolation weights
			int w0 = _octWeights[o] * (pow( sin( (fraction + 2) * Math::Pi / 3 ), 2 ) / 1.5f) * SCALE;
			int w1 = _octWeights[o] * (pow( sin( (fraction + 1) * Math::Pi / 3 ), 2 ) / 1.5f) * SCALE;
			int w2 = _octWeights[o] * (pow( sin( fraction * Math::Pi / 3 ), 2 ) / 1.5f) * SCALE;
			int frame0 = frame & (NOISE_FRAMES - 1);
			int frame1 = (frame + 1) & (NOISE_FRAMES - 1);
			int frame2 = (frame + 2) & (NOISE_FRAMES - 1);
			for( int i = 0; i < NOISE_SIZE_SQ; i++ )
			{
				_octave_noise[i + NOISE_SIZE_SQ * o] = (w0 * _noise[i + NOISE_SIZE_SQ * frame0]
				                                      + w1 * _noise[i + NOISE_SIZE_SQ * frame1]
				                                      + w2 * _noise[i + NOISE_SIZE_SQ * frame2]) / SCALE;
			}

			timefactor *= 1.3f;
			// Generate octave texture
			glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, _octaveTex[o], 0 );
			glClear( GL_COLOR_BUFFER_BIT );
			
			float w0f = (float)w0 / SCALE;
			float w1f = (float)w1 / SCALE;
			float w2f = (float)w2 / SCALE;
			glBindTexture( GL_TEXTURE_2D, _noiseTex[frame0] );
			glColor3f( w0f, w0f, w0f );
			glBegin( GL_QUADS );
				glTexCoord2f( 0, 0 ); glVertex2f( 0, 0 );
				glTexCoord2f( 1, 0 ); glVertex2f( 1, 0 );
				glTexCoord2f( 1, 1 ); glVertex2f( 1, 1 );
				glTexCoord2f( 0, 1 ); glVertex2f( 0, 1 );
			glEnd();
			glBindTexture( GL_TEXTURE_2D, _noiseTex[frame1] );
			glColor3f( w1f, w1f, w1f );
			glBegin( GL_QUADS );
				glTexCoord2f( 0, 0 ); glVertex2f( 0, 0 );
				glTexCoord2f( 1, 0 ); glVertex2f( 1, 0 );
				glTexCoord2f( 1, 1 ); glVertex2f( 1, 1 );
				glTexCoord2f( 0, 1 ); glVertex2f( 0, 1 );
			glEnd();
			glBindTexture( GL_TEXTURE_2D, _noiseTex[frame2] );
			glColor3f( w2f, w2f, w2f );
			glBegin( GL_QUADS );
				glTexCoord2f( 0, 0 ); glVertex2f( 0, 0 );
				glTexCoord2f( 1, 0 ); glVertex2f( 1, 0 );
				glTexCoord2f( 1, 1 ); glVertex2f( 1, 1 );
				glTexCoord2f( 0, 1 ); glVertex2f( 0, 1 );
			glEnd();
		}
		// Pack octaves
		int octavepack = 0;
		for( int o = 0; o < _octaves; o += PACK_SIZE )
		{
			for( int y = 0; y < PACKED_SIZE; y++ )
			{
				for( int x = 0; x < PACKED_SIZE; x++ )
				{
					int value = _octave_noise[(o + 3) * NOISE_SIZE_SQ + ( y & (NOISE_SIZE - 1)) * NOISE_SIZE + (x & (NOISE_SIZE - 1))]
					            + mapSample( x, y, 3, o )
					            + mapSample( x, y, 2, o + 1 )
					            + mapSample( x, y, 1, o + 2 );
					_packed_noise[octavepack * PACKED_SIZE_SQ + y * PACKED_SIZE + x] = value;
				}
			}
			octavepack++;
		}
		// Create GPU heightmap
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, _FBO2 );
		glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, _heightMap->getTexObject(), 0 );
		glViewport( 0, 0, 1024, 1024 );
		glClear( GL_COLOR_BUFFER_BIT );

		glColor3f( 1.0f, 1.0f, 1.0f );
		for( int o = 0; o < _octaves; o++ )
		{
			glBindTexture( GL_TEXTURE_2D, _octaveTex[o] );
			float size = 1 << o;

			glBegin( GL_QUADS );
				glTexCoord2f( 0,    0 ); glVertex2f( 0, 0 );
				glTexCoord2f( size, 0 ); glVertex2f( 1, 0 );
				glTexCoord2f( size, size ); glVertex2f( 1, 1 );
				glTexCoord2f( 0,    size ); glVertex2f( 0, 1 );
			glEnd();
		}

		// Create GPU normalmap
		glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, _normalMap->getTexObject(), 0 );
		glClear( GL_COLOR_BUFFER_BIT );

		glUseProgram( _normalProgram );
		glDisable( GL_BLEND );
		glBindTexture( GL_TEXTURE_2D, _heightMap->getTexObject() );

		glBegin( GL_QUADS );
			glTexCoord2f( 0, 0 ); glVertex2f( 0, 0 );
			glTexCoord2f( 1, 0 ); glVertex2f( 1, 0 );
			glTexCoord2f( 1, 1 ); glVertex2f( 1, 1 );
			glTexCoord2f( 0, 1 ); glVertex2f( 0, 1 );
		glEnd();

		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
		glBindTexture( GL_TEXTURE_2D, _normalMap->getTexObject() );
		glGenerateMipmapEXT( GL_TEXTURE_2D );

		glPopAttrib();
	}
	void NoiseResource::calcOctWeights()
	{
		float sum = 0.0f;
		float weight = 1.0f;
		// Compute weights
		for( int i = 0; i < _octaves; i++ )
		{
			_octWeights[i] = weight;
			sum += weight;
			weight *= _falloff;
		}
		// Set maximal height
		for( int i = 0; i < _octaves; i++ )
		{
			_octWeights[i] /= sum;
		}
	}

	void NoiseResource::initializationFunc()
	{
	}
	void NoiseResource::releaseFunc()
	{
	}

	void NoiseResource::initDefault()
	{
	}
	void NoiseResource::release()
	{
	}
	bool NoiseResource::load( const char *data, int size )
	{
		return true;
	}

	int NoiseResource::getParami( int param )
	{
		return 0;
	}
	bool NoiseResource::setParami( int param, int value )
	{
		return true;
	}

	void NoiseResource::setTime( float time )
	{
		_time = time;
		// Rebuild noise data
		calcNoise();
	}
	float NoiseResource::getHeight( float x, float y )
	{
		x *= PACKED_SIZE;
		y *= PACKED_SIZE;
		int packCount = (_octaves - 1) / PACK_SIZE;
		int factor = 1 << (packCount * PACK_SIZE);
		float height = 0.0f;
		for( int i = packCount; i >= 0; i-- )
		{
			_current = _packed_noise + (PACKED_SIZE_SQ * i);
			height += getTexel( x * factor, y * factor );
			factor >>= PACK_SIZE;
		}
		return height;
	}
}
