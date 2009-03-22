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

#ifndef _Horde3DWater_noise_H_
#define _Horde3DWater_noise_H_

#include "egResource.h"
#include "egTextures.h"
#include "egTextures.h"

namespace Horde3DWater
{
	const int RT_NoiseResource = 148;

	struct NoiseResParams
	{
		enum List
		{
			Octaves = 10000,
			NormalMap
		};
	};

	// Perlin noise constants
	const int MAX_OCTAVES = 32;
	const int PACK_SIZE = 4;

	const int NOISE_FRAMES = 256;

	const int NOISE_SIZE = 16;
	const int NOISE_SIZE_SQ = NOISE_SIZE * NOISE_SIZE;
	const int NOISE_SIZE_M1 = NOISE_SIZE - 1;

	const int PACKED_SIZE = NOISE_SIZE << (PACK_SIZE - 1);
	const int PACKED_SIZE_SQ = PACKED_SIZE * PACKED_SIZE;
	const int PACKED_SIZE_M1 = PACKED_SIZE - 1;

	const int SCALE = 0x10000;

	class NoiseResource : public Resource
	{
	protected:
		int    _octaves;
		float  _falloff;
		float  _time;

		// CPU noise
		int    _noise[NOISE_SIZE_SQ * NOISE_FRAMES];
		int    _octave_noise[NOISE_SIZE_SQ * MAX_OCTAVES];
		int    _packed_noise[PACKED_SIZE_SQ * MAX_OCTAVES / PACK_SIZE];
		int   *_current;
		float  _octWeights[MAX_OCTAVES];
		// GPU noise
		uint32 _noiseTex[NOISE_FRAMES];
		uint32 _octaveTex[MAX_OCTAVES];
		uint32 _normalProgram;
		uint32 _FBO;
		uint32 _FBO2;

		void initNoise();
		void calcNoise();
		void calcOctWeights();
		inline int mapSample( int x, int y, int upSamplePower, int octave );
		inline float getTexel( float x, float y );
	public:
		PTextureResource _heightMap;
		PTextureResource _normalMap;
		NoiseResource( const std::string &name, int flags, int octaves = 8 );
		~NoiseResource();

		static void initializationFunc();
		static void releaseFunc();
		static Resource *factoryFunc( const std::string &name, int flags )
			{ return new NoiseResource( name, flags ); }

		virtual void initDefault();
		virtual void release();
		virtual bool load( const char *data, int size );

		virtual int getParami( int param );
		virtual bool setParami( int param, int value );

		void setTime( float time );
		float getHeight( float x, float y );
	};

	typedef SmartResPtr< NoiseResource > PNoiseResource;

	inline int NoiseResource::mapSample( int x, int y, int upSamplePower, int octave )
	{
		int magnitude = 1 << upSamplePower;
		int px = (x >> upSamplePower) & NOISE_SIZE_M1;
		int py = (y >> upSamplePower) & NOISE_SIZE_M1;
		int fx = x & (magnitude - 1);
		int fy = y & (magnitude - 1);
		int fx_m = magnitude - fx;
		int fy_m = magnitude - fy;

		int ooffset = octave * NOISE_SIZE_SQ;
		int o = fx_m * fy_m * _octave_noise[ooffset + py                         * NOISE_SIZE + px                        ]
		      + fx   * fy_m * _octave_noise[ooffset + py                         * NOISE_SIZE + ((px + 1) & NOISE_SIZE_M1)]
		      + fx_m * fy   * _octave_noise[ooffset + ((py + 1) & NOISE_SIZE_M1) * NOISE_SIZE + px                        ]
		      + fx   * fy   * _octave_noise[ooffset + ((py + 1) & NOISE_SIZE_M1) * NOISE_SIZE + ((px + 1) & NOISE_SIZE_M1)];

		return o / (magnitude * magnitude);
	}
	inline float NoiseResource::getTexel( float x, float y )
	{
		if (!_current) return 0.0f;
		int ix = x;
		int iy = y;
		float fx = x - ix;
		float fy = y - iy;
		float fx_m = 1 - fx;
		float fy_m = 1 - fy;
		int v1 = _current[(iy & PACKED_SIZE_M1)       * PACKED_SIZE + (ix & PACKED_SIZE_M1)];
		int v2 = _current[(iy & PACKED_SIZE_M1)       * PACKED_SIZE + ((ix + 1) & PACKED_SIZE_M1)];
		int v3 = _current[((iy + 1) & PACKED_SIZE_M1) * PACKED_SIZE + (ix & PACKED_SIZE_M1)];
		int v4 = _current[((iy + 1) & PACKED_SIZE_M1) * PACKED_SIZE + ((ix + 1) & PACKED_SIZE_M1)];
		return (fx_m * fy_m * v1
		      + fx   * fy_m * v2
		      + fx_m * fy   * v3
		      + fx   * fy   * v4) / SCALE;
	}

}

#endif
