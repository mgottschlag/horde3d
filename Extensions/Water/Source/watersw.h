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

#ifndef _Horde3DWater_watersw_H_
#define _Horde3DWater_watersw_H_

#include "water.h"

namespace Horde3DWater
{
	extern const char *vsWaterSWDebugView;
	extern const char *fsWaterSWDebugView;

	class WaterNodeSW : public WaterNode
	{
	protected:
		// Geometry data
		uint32            _vertexBuffer, _indexBuffer;

		float* vertices;

		// Geometry
		void createBuffers();
		void destroyBuffers();
		void updateBuffers( float x1 = 0, float y1 = 0, float x2 = 1, float y2 = 1 );
		virtual void render();
	public:
		static ShaderCombination debugViewShader;

		WaterNodeSW( const WaterNodeTpl &waterTpl );
		~WaterNodeSW();

		bool setParami( int param, int value );

		BoundingBox *getLocalBBox() { return &_localBBox; }
	};
}

#endif
