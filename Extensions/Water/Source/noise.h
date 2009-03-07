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

#ifndef _Horde3DWater_noise_H_
#define _Horde3DWater_noise_H_

namespace Horde3DWater
{
	class NoiseGenerator
	{
	private:
		float *noisedata;
		float *currentframe;

		int width, height, frames;
		float length;
		int currentframeidx;

		float getInterpolated( float *noise, int x, int y, int z, float dx, float dy, float dz );
	public:
		NoiseGenerator();
		~NoiseGenerator();

		int initialize( int width, int height, int frames, float length );

		void setFrame( int frame );
		int getFrame();

		void update( float time );
		float getHeightI( int x, int y );
		float getHeight( float x, float y );
		float getValue( float x, float y, float *normal = 0 );
	};
}

#endif
