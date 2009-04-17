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

#include "water.h"
#include "watersw.h"
#include "watergpu.h"
#include "egModules.h"
#include "egMaterial.h"
#include "utOpenGL.h"

namespace Horde3DWater
{
	WaterNode::WaterNode( const WaterNodeTpl &waterTpl ) : SceneNode( waterTpl ),
		_matRes( waterTpl.matRes ), _noiseRes( waterTpl.noiseRes ),
		_useGPU(waterTpl.useGPU)
	{
		_renderable = true;
		_localBBox.getMinCoords() = Vec3f( -100, 0, -100 );
		_localBBox.getMaxCoords() = Vec3f( 100, 100, 100 );
	}
	WaterNode::~WaterNode()
	{
	}

	SceneNodeTpl *WaterNode::parsingFunc( std::map< std::string, std::string > &attribs )
	{
		return 0;
	}
	SceneNode *WaterNode::factoryFunc( const SceneNodeTpl &nodeTpl )
	{
		if( nodeTpl.type != SNT_WaterNode ) return 0x0;

		WaterNodeTpl *tpl = (WaterNodeTpl *)&nodeTpl;
		printf("GPU: %d\n", tpl->useGPU);
		if (tpl->useGPU)
			return new WaterNodeGPU( *tpl );
		else
			return new WaterNodeSW( *tpl );
	}
	void WaterNode::renderFunc( const std::string &shaderContext, const std::string &theClass, bool debugView,
		const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order, int occSet )
	{
		CameraNode *curCam = Modules::renderer().getCurCamera();
		if( curCam == 0x0 ) return;

		Modules::renderer().setMaterial( 0x0, "" );

		// Loop through water queue
		for( uint32 i = 0, s = (uint32)Modules::sceneMan().getRenderableQueue().size(); i < s; ++i )
		{
			if( Modules::sceneMan().getRenderableQueue()[i].type != SNT_WaterNode ) continue;

			WaterNode *water = (WaterNode *)Modules::sceneMan().getRenderableQueue()[i].node;
			if( !debugView )
			{
				if( !water->_matRes->isOfClass( theClass ) ) continue;
				if( !Modules::renderer().setMaterial( water->_matRes, shaderContext ) ) continue;
			}
			else
			{
				if (water->_useGPU)
					Modules::renderer().setShader( &WaterNodeGPU::debugViewShader );
				else
					Modules::renderer().setShader( &WaterNodeSW::debugViewShader );
			}

			// World transformation
			water->_absTransInv = water->_absTrans.inverted();
			ShaderCombination *curShader = Modules::renderer().getCurShader();
			if( curShader->uni_worldMat >= 0 )
			{
				glUniformMatrix4fv( curShader->uni_worldMat, 1, false, &water->_absTrans.x[0] );
			}
			if( curShader->uni_worldNormalMat >= 0 )
			{
				Matrix4f normalMat4 = water->_absTransInv.transposed();
				float normalMat[9] = { normalMat4.x[0], normalMat4.x[1], normalMat4.x[2],
									normalMat4.x[4], normalMat4.x[5], normalMat4.x[6],
									normalMat4.x[8], normalMat4.x[9], normalMat4.x[10] };
				glUniformMatrix3fv( curShader->uni_worldNormalMat, 1, false, normalMat );
			}

			water->render();
		}
	}

	bool WaterNode::canAttach( SceneNode &parent )
	{
		return true;
	}
	int WaterNode::getParami( int param )
	{
		switch( param )
		{
		case WaterNodeParams::MaterialRes:
			if( _matRes != 0x0 ) return _matRes->getHandle();
			else return 0;
		default:
			return SceneNode::getParami( param );
		}
	}
	bool WaterNode::setParami( int param, int value )
	{
		Resource *res;
		switch( param )
		{
		case WaterNodeParams::MaterialRes:
			res = Modules::resMan().resolveResHandle( value );
			if( res == 0x0 || res->getType() != ResourceTypes::Material )
			{
				Modules::log().writeDebugInfo( "Invalid Material resource for Water node %i", _handle );
				return false;
			}
			_matRes = (MaterialResource *)res;
			// TODO: Set noise texture
			return true;
		default:
			return SceneNode::setParami( param, value );
		}
	}
}
