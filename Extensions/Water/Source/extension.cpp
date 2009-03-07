// *************************************************************************************************
//
// Horde3D Nature Extension
// --------------------------------------------------------
// Copyright (C) 2006-2008 Nicolas Schulz and Volker Wiendl
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

#include "water.h"
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
		Modules::sceneMan().registerType( SNT_WaterNode, "Water",
			WaterNode::parsingFunc, WaterNode::factoryFunc, WaterNode::renderFunc );

		// Upload default shader used for debug view
		Modules::renderer().uploadShader(
			vsWaterDebugView, fsWaterDebugView, WaterNode::debugViewShader );
		
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

	
	DLLEXP NodeHandle addWaterNode( NodeHandle parent, const char *name,
									  ResHandle materialRes )
	{
		SceneNode *parentNode = Modules::sceneMan().resolveNodeHandle( parent );
		if( parentNode == 0x0 ) return 0;
		
		Resource *matRes =  Modules::resMan().resolveResHandle( materialRes );
		if( matRes == 0x0 || matRes->getType() != ResourceTypes::Material ) return 0;
		
		Modules::log().writeInfo( "Adding Water node '%s'", safeStr( name ).c_str() );
		
		WaterNodeTpl tpl( safeStr( name ), (MaterialResource *)matRes );
		SceneNode *sn = Modules::sceneMan().findType( SNT_WaterNode )->factoryFunc( tpl );
		return Modules::sceneMan().addNode( sn, *parentNode );
	}
}

