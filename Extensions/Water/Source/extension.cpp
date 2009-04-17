// *************************************************************************************************
//
// Horde3D Water Extension
// --------------------------------------------------------
// Copyright (C) 2009 Nicolas Schulz, Volker Wiendl, Mathias Gottschlag
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

#include "utPlatform.h"

#include "watersw.h"
#include "watergpu.h"
#include "noise.h"
#include "egModules.h"

using namespace std;

namespace Horde3DWater
{
	const char *getExtensionName()
	{
		return "Water";
	}
	
	
	bool initExtension()
	{
		Modules::resMan().registerType( RT_NoiseResource, "Noise", NoiseResource::initializationFunc,
			NoiseResource::releaseFunc, NoiseResource::factoryFunc );
		Modules::sceneMan().registerType( SNT_WaterNode, "Water",
			WaterNode::parsingFunc, WaterNode::factoryFunc, WaterNode::renderFunc );

		// Upload default shader used for debug view
		Modules::renderer().uploadShader(
			vsWaterSWDebugView, fsWaterSWDebugView, WaterNodeSW::debugViewShader );
		Modules::renderer().uploadShader(
			vsWaterGPUDebugView, fsWaterGPUDebugView, WaterNodeGPU::debugViewShader );

		return true;
	}

	
	void releaseExtension()
	{
	}

	
	string safeStr( const char *str )
	{
		if( str != 0x0 ) return str;
		else return "";
	}

	DLLEXP ResHandle addNoise( const char *name, int octaves )
	{
		NoiseResource *noiseRes = new NoiseResource( safeStr( name ), 0, octaves );

		ResHandle res = Modules::resMan().addNonExistingResource( *noiseRes, true );
		if( res == 0 )
		{
			Modules::log().writeDebugInfo( "Failed to add resource in addNoise; might be the name is already in use?", res );
			delete noiseRes;
		}

		return res;
	}
	DLLEXP void setNoiseTime( ResHandle noise, float time )
	{
		Resource *res = Modules::resMan().resolveResHandle( noise );

		if( res != 0x0 && res->getType() == RT_NoiseResource )
			((NoiseResource *)res)->setTime( time );
		else
		{
			Modules::log().writeDebugInfo( "Invalid Noise resource handle %i in setNoiseTime", noise );
		}
	}
	DLLEXP float getNoiseHeight( ResHandle noise, float x, float z)
	{
		Resource *res = Modules::resMan().resolveResHandle( noise );

		if( res != 0x0 && res->getType() == RT_NoiseResource )
			return ((NoiseResource *)res)->getHeight( x, z );
		else
		{
			Modules::log().writeDebugInfo( "Invalid Noise resource handle %i in setNoiseTime", noise );
			return 0.0f;
		}
	}

	DLLEXP int getGPUWaterSupported( void )
	{
		return 1;
	}

	DLLEXP NodeHandle addWaterNode( NodeHandle parent, const char *name,
	                                ResHandle noiseRes, ResHandle materialRes )
	{
		SceneNode *parentNode = Modules::sceneMan().resolveNodeHandle( parent );
		if( parentNode == 0x0 ) return 0;

		Resource *matRes =  Modules::resMan().resolveResHandle( materialRes );
		if( matRes == 0x0 || matRes->getType() != ResourceTypes::Material ) return 0;
		Resource *noiseResPtr =  Modules::resMan().resolveResHandle( noiseRes );
		if( noiseResPtr == 0x0 || noiseResPtr->getType() != RT_NoiseResource ) return 0;

		Modules::log().writeInfo( "Adding Water node '%s'", safeStr( name ).c_str() );

		WaterNodeTpl tpl( safeStr( name ), (MaterialResource *)matRes, (NoiseResource *)noiseResPtr, false );
		SceneNode *sn = Modules::sceneMan().findType( SNT_WaterNode )->factoryFunc( tpl );
		return Modules::sceneMan().addNode( sn, *parentNode );
	}

	DLLEXP NodeHandle addWaterNodeGPU( NodeHandle parent, const char *name,
	                                   ResHandle noiseRes, ResHandle materialRes )
	{
		SceneNode *parentNode = Modules::sceneMan().resolveNodeHandle( parent );
		if( parentNode == 0x0 ) return 0;

		Resource *matRes =  Modules::resMan().resolveResHandle( materialRes );
		if( matRes == 0x0 || matRes->getType() != ResourceTypes::Material ) return 0;
		Resource *noiseResPtr =  Modules::resMan().resolveResHandle( noiseRes );
		if( noiseResPtr == 0x0 || noiseResPtr->getType() != RT_NoiseResource ) return 0;

		Modules::log().writeInfo( "Adding WaterGPU node '%s'", safeStr( name ).c_str() );

		WaterNodeTpl tpl( safeStr( name ), (MaterialResource *)matRes, (NoiseResource *)noiseResPtr, true );
		SceneNode *sn = Modules::sceneMan().findType( SNT_WaterNode )->factoryFunc( tpl );
		return Modules::sceneMan().addNode( sn, *parentNode );
	}
}

