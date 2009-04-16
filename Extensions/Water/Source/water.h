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

#ifndef _Horde3DWater_water_H_
#define _Horde3DWater_water_H_

#include "noise.h"
#include "egScene.h"

namespace Horde3DWater
{
	const int SNT_WaterNode = 149;

	struct WaterNodeParams
	{
		enum List
		{
			MaterialRes = 10000,
			NoiseRes,
			GridWidth,
			GridHeight
		};
	};

	const int GRID_SIZE = 64;

	extern const char *vsWaterDebugView;
	extern const char *fsWaterDebugView;

	struct WaterNodeTpl : public SceneNodeTpl
	{
		PMaterialResource matRes;
		PNoiseResource    noiseRes;
		int               gridWidth;
		int               gridHeight;

		WaterNodeTpl( const std::string &name, MaterialResource *matRes,
		              NoiseResource *noiseRes ) :
			SceneNodeTpl( SNT_WaterNode, name ), matRes( matRes ),
			noiseRes(noiseRes), gridWidth(64), gridHeight(64)
		{
		}
	};

	class WaterNode : public SceneNode
	{
	protected:
		// Settings
		PMaterialResource _matRes;
		PNoiseResource    _noiseRes;
		int               _gridWidth;
		int               _gridHeight;

		// Geometry data
		uint32            _vertexBuffer, _indexBuffer;
		BoundingBox       _localBBox;

		// Frustum data
		Frustum           _frustum;
		Matrix4f          _viewMat;
		Matrix4f          _projMat;
		Matrix4f          _absTransInv;

		float* vertices;
		unsigned short* indices;

		WaterNode( const WaterNodeTpl &waterTpl );

		// Geometry
		void createBuffers();
		void destroyBuffers();
		void updateBuffers( float x1 = 0, float y1 = 0, float x2 = 1, float y2 = 1 );
		void render();
	public:
		static ShaderCombination debugViewShader;

		~WaterNode();

		static SceneNodeTpl *parsingFunc( std::map< std::string, std::string > &attribs );
		static SceneNode *factoryFunc( const SceneNodeTpl &nodeTpl );
		static void renderFunc( const std::string &shaderContext, const std::string &theClass, bool debugView,
			const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order, int occSet );

		bool canAttach( SceneNode &parent );
		int getParami( int param );
		bool setParami( int param, int value );

		BoundingBox *getLocalBBox() { return &_localBBox; }
	};
}

#endif
