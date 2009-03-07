// *************************************************************************************************
//
// Horde3D Water Extension
// --------------------------------------------------------
// Copyright (C) 2006-2008 Nicolas Schulz, Volker Wiendl, Mathias Gottschlag
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
#include "egPrerequisites.h"
#include "utMath.h"
#include "egMaterial.h"
#include "egTextures.h"
#include "egScene.h"


namespace Horde3DWater
{
	const int SNT_WaterNode = 149;

	extern const char *vsWaterDebugView;
	extern const char *fsWaterDebugView;
	
	struct WaterNodeParams
	{
		enum List
		{
			MaterialRes = 10000,
			MeshQuality
		};
	};
	
	struct WaterNodeTpl : public SceneNodeTpl
	{
		PMaterialResource  matRes;
		float              meshQuality;
		int                blockSize;

		WaterNodeTpl( const std::string &name, MaterialResource *matRes ) :
			SceneNodeTpl( SNT_WaterNode, name ), matRes( matRes ),
			meshQuality( 50.0f )
		{
		}
	};

	
	class WaterNode : public SceneNode
	{
	protected:
		
		PMaterialResource _materialRes;
		
		float            *_heightArray;
		uint32            _vertexBuffer, _indexBuffer;

		BoundingBox       _localBBox;

		NoiseGenerator noise;
		float lastframetime;

		// Camera information
		bool camerabuilt;
		Frustum frustum;
		Matrix4f viewMat;
		Matrix4f projMat;

		WaterNode( const WaterNodeTpl &waterTpl );
		
		void renderWater( float x, float z );
		
		void createBuffers( int sizeX, int sizeZ, uint32 *vertexBuffer, uint32 *indexBuffer,
			bool fill = false, float x1 = 0, float y1 = 0, float x2 = 1, float y2 = 1 );
		void createBuffers( bool fill = false, float x1 = 0, float y1 = 0, float x2 = 1, float y2 = 1 );
		void destroyBuffers( void );
		
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
		float getParamf( int param );
		bool setParamf( int param, float value );

		BoundingBox *getLocalBBox() { return &_localBBox; }
	};
}

#endif // _Horde3DWater_water_H_
