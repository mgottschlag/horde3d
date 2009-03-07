// *************************************************************************************************
//
// Horde3D Water Extension
// --------------------------------------------------------
// Copyright (C) 2006-2008 Mathias Gottschlag
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
#include "utMath.h"

#include <cstdlib>
#include <cstdio>
#include <cmath>

namespace Horde3DWater
{
	NoiseGenerator::NoiseGenerator()
	{
		noisedata = 0;
		currentframe = 0;
		currentframeidx = 0;
	}
	NoiseGenerator::~NoiseGenerator()
	{
		if (noisedata) delete[] noisedata;
		if (currentframe) delete[] currentframe;
	}

	int NoiseGenerator::initialize( int width, int height, int frames, float length )
	{
		this->width = width;
		this->height = height;
		this->frames = frames;
		this->length = length;

		// Delete old data
		if ( noisedata ) delete[] noisedata;
		if ( currentframe ) delete[] currentframe;
		noisedata = 0;
		currentframe = 0;
		if ( (width == 0) || (height == 0) || (frames == 0) ) return 0;

		// Create 3D noise
		float noise[16 * 16 * 16];
		for ( int i = 0; i < 16 * 16 * 16; i++ )
		{
			noise[i] = (float)rand() / RAND_MAX;
		}
		noisedata = new float[width * height * frames];
		if ( !noisedata ) return -1;
		// 4 octaves of noise
		for ( int i = 0; i < width * height * frames; i++ )
		{
			float alpha = 0.7;
			float weight = 0.5;

			noisedata[i] = 0;

			int x = i % width;
			int y = (i / width) % height;
			int z = i / width / height;
			//int srcz = z % 16;

			// 1st octave
			int factor = width / 16;
			float dx = (float)(x % factor) / factor * M_PI;
			float dy = (float)(y % factor) / factor * M_PI;
			float dz = (float)(z % factor) / factor * M_PI;
			int srcx = (x / factor) % 16;
			int srcy = (y / factor) % 16;
			int srcz = (z / factor) % 16;
			noisedata[i] = getInterpolated( noise, srcx, srcy, srcz, dx, dy, dz ) * weight;
			// 2nd octave
			factor = factor / 2;
			weight *= alpha;
			dx = (float)(x % factor) / factor * M_PI;
			dy = (float)(y % factor) / factor * M_PI;
			dz = (float)(z % factor) / factor * M_PI;
			srcx = (x / factor) % 16;
			srcy = (y / factor) % 16;
			srcz = (z / factor) % 16;
			noisedata[i] = noisedata[i] + getInterpolated( noise, srcx, srcy, srcz, dx, dy, dz ) * weight;
			// 3rd octave
			factor = factor / 2;
			weight *= alpha;
			dx = (float)(x % factor) / factor * M_PI;
			dy = (float)(y % factor) / factor * M_PI;
			dz = (float)(z % factor) / factor * M_PI;
			srcx = (x / factor) % 16;
			srcy = (y / factor) % 16;
			srcz = (z / factor) % 16;
			noisedata[i] = noisedata[i] + getInterpolated( noise, srcx, srcy, srcz, dx, dy, dz ) * weight;
			// 4th octave
			factor = factor / 2;
			weight *= alpha;
			dx = (float)(x % factor) / factor * M_PI;
			dy = (float)(y % factor) / factor * M_PI;
			dz = (float)(z % factor) / factor * M_PI;
			srcx = (x / factor) % 16;
			srcy = (y / factor) % 16;
			srcz = (z / factor) % 16;
			noisedata[i] = noisedata[i] + getInterpolated( noise, srcx, srcy, srcz, dx, dy, dz ) * weight;
		}

		return 0;
	}

	float NoiseGenerator::getInterpolated( float *noise, int x, int y, int z, float dx, float dy, float dz )
	{
		float value1 = noise[(x % 16)       + (y % 16) * 16       + (z % 16) * 16 * 16];
		float value2 = noise[((x + 1) % 16) + (y % 16) * 16       + (z % 16) * 16 * 16];
		float value3 = noise[(x % 16)       + ((y + 1) % 16) * 16 + (z % 16) * 16 * 16];
		float value4 = noise[((x + 1) % 16) + ((y + 1) % 16) * 16 + (z % 16) * 16 * 16];
		float value5 = noise[(x % 16)       + (y % 16) * 16       + ((z + 1) % 16) * 16 * 16];
		float value6 = noise[((x + 1) % 16) + (y % 16) * 16       + ((z + 1) % 16) * 16 * 16];
		float value7 = noise[(x % 16)       + ((y + 1) % 16) * 16 + ((z + 1) % 16) * 16 * 16];
		float value8 = noise[((x + 1) % 16) + ((y + 1) % 16) * 16 + ((z + 1) % 16) * 16 * 16];
		float factor1 = cos(dx) / 2 + 0.5;
		float factor2 = cos(dy) / 2 + 0.5;
		float factor3 = cos(dz) / 2 + 0.5;
		float tmp1 = value1 * factor1 + value2 * (1 - factor1);
		float tmp2 = value3 * factor1 + value4 * (1 - factor1);
		float tmp3 = tmp1 * factor2 + tmp2 * (1 - factor2);
		tmp1 = value5 * factor1 + value6 * (1 - factor1);
		tmp2 = value7 * factor1 + value8 * (1 - factor1);
		float tmp4 = tmp1 * factor2 + tmp2 * (1 - factor2);
		return tmp3 * factor3 + tmp4 * (1 - factor3);
	}

	void NoiseGenerator::setFrame( int frame )
	{
		currentframeidx = frame % frames;
	}
	int NoiseGenerator::getFrame()
	{
		return currentframeidx;
	}

	void NoiseGenerator::update( float time )
	{
	}
	float NoiseGenerator::getHeightI( int x, int y )
	{
		while (x < 0) x += width;
		while (y < 0) y += height;
		x %= width;
		y %= height;
		return noisedata[x + y * width + currentframeidx * width * height];
	}
	float NoiseGenerator::getHeight( float x, float y )
	{
		float dx = x - (int)x;
		float dy = y - (int)y;
		float h1 = getHeightI( (int)x, (int)y );
		float h2 = getHeightI( (int)x + 1, (int)y );
		float h3 = getHeightI( (int)x, (int)y + 1 );
		float h4 = getHeightI( (int)x + 1, (int)y + 1 );
		h1 = h1 * (1 - dx) + h2 * dx;
		h3 = h3 * (1 - dx) + h4 * dx;
		return h1 * (1 - dy) + h3 * dy;
	}
	float NoiseGenerator::getValue( float x, float y, float *normal )
	{
		if (!noisedata) return 0;

		while (x < 0) x += width;
		while (y < 0) y += height;
		int x2 = (int)x % width;
		int z2 = (int)y % height;
		if ( normal )
		{
			float h1 = getValue( x - 1, y );
			float h2 = getValue( x + 1, y );
			float h3 = getValue( x, y - 1 );
			float h4 = getValue( x, y + 1 );

			Vec3f v1( 2.0, h2 - h1, 0.0 );
			Vec3f v2( 0.0, h4 - h3, 2.0 );
			Vec3f n = v1.cross( v2 ).normalized();
			//n.y = 0;
			normal[0] = n.x;
			normal[1] = n.y;
			normal[2] = n.z;
		}
		return getHeight( x, y );
	}
}
