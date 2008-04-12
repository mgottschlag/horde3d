// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2008 Nicolas Schulz
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

#ifndef _daeLibVisualScenes_H_
#define _daeLibVisualScenes_H_

#include "utXMLParser.h"
#include "daeLibAnimations.h"
#include <string>
#include <vector>
#include <map>
using namespace std;


struct DaeTransformation
{
	string					sid;
	float					values[16];


	DaeTransformation()
	{
		// Identity matrix
		for( unsigned int i = 0; i < 16; ++i ) values[i] = 0;
		for( unsigned int i = 0; i < 4; ++i ) values[i * 4 + i] = 1;
	}
};


struct DaeInstance
{
	string					url;
	map< string, string >	materialBindings;
};


struct DaeNode
{
	string					id, sid;
	string					name;
	bool					joint;
	DaeTransformation		transMat;		// Single baked matrix, relative to parent
	vector< DaeNode * >		children;
	vector< DaeInstance >	instances;
	

	bool parse( const XMLNode &nodeNode )
	{
		id = nodeNode.getAttribute( "id", "" );
		if( id == "" ) return false;
		name = nodeNode.getAttribute( "name", "" );
		sid = nodeNode.getAttribute( "sid", "" );
		
		if( strcmp( nodeNode.getAttribute( "type", "" ), "JOINT" ) == 0 ) joint = true;
		else joint = false;
		
		// Parse transformations
		unsigned int numMatrices = 0;
		int nodeItr1 = 0;
		XMLNode node1 = nodeNode.getChildNode( nodeItr1 );
		if( !node1.isEmpty() && node1.getName() != 0x0 )
		{
			if( strcmp( node1.getName(), "matrix" ) == 0 && numMatrices < 1 )
			{
				++numMatrices;
				transMat.sid = node1.getAttribute( "sid", "" );
				
				unsigned int pos = 0;
				char *s = (char *)node1.getText();
				if( s == 0x0 ) return false;
				for( int i = 0; i < 16; ++i )
				{
					float f;
					parseFloat( s, pos, f );
					transMat.values[i] = f;
				}
			}
			else
			{
				if( strcmp( node1.getName(), "translate" ) == 0 || strcmp( node1.getName(), "rotate" ) == 0 ||
					strcmp( node1.getName(), "scale" ) == 0 || strcmp( node1.getName(), "skew" ) == 0 ||
					strcmp( node1.getName(), "matrix" ) == 0 )
				{
					log( "Warning: Expected single baked matrix for node; use appropriate export options" );
				}
			}
		}

		// Parse instances
		nodeItr1 = 0;
		node1 = nodeNode.getChildNode( nodeItr1 );
		while( !node1.isEmpty() && node1.getName() != 0x0 )
		{
			if( strcmp( node1.getName(), "instance_geometry" ) == 0 ||
				strcmp( node1.getName(), "instance_controller" ) == 0 )
			{
				string url = node1.getAttribute( "url", "" );
				removeGate( url );

				if( url != "" )
				{
					instances.push_back( DaeInstance() );
					DaeInstance &inst = instances[instances.size() - 1];
					
					inst.url = url;

					// Find material bindings
					XMLNode node2 = node1.getChildNode( "bind_material" );
					if( !node2.isEmpty() )
					{
						XMLNode node3 = node2.getChildNode( "technique_common" );
						if( !node3.isEmpty() )
						{
							int nodeItr4 = 0;
							XMLNode node4 = node3.getChildNode( "instance_material", nodeItr4 );
							while( !node4.isEmpty() )
							{
								string s = node4.getAttribute( "target", "" );
								removeGate( s );
								inst.materialBindings[node4.getAttribute( "symbol", "" )] = s;

								node4 = node3.getChildNode( "instance_material", ++nodeItr4 );
							}
						}
					}
				}
			}
			
			node1 = nodeNode.getChildNode( ++nodeItr1 );
		}

		// Parse children
		nodeItr1 = 0;
		node1 = nodeNode.getChildNode( "node", nodeItr1 );
		while( !node1.isEmpty() )
		{
			DaeNode *node = new DaeNode();
			if( node->parse( node1 ) ) children.push_back( node );
			else delete node;

			node1 = nodeNode.getChildNode( "node", ++nodeItr1 );
		}
		
		return true;
	}
};


struct DaeVisualScene
{
	string					id;
	vector< DaeNode * >		nodes;

	
	~DaeVisualScene()
	{
		for( unsigned int i = 0; i < nodes.size(); ++i ) delete nodes[i];
	}


	bool parse( const XMLNode &visSceneNode )
	{
		id = visSceneNode.getAttribute( "id", "" );
		if( id == "" ) return false;
		
		int nodeItr1 = 0;
		XMLNode node1 = visSceneNode.getChildNode( "node", nodeItr1 );
		while( !node1.isEmpty() )
		{
			DaeNode *node = new DaeNode();
			if( node->parse( node1 ) ) nodes.push_back( node );
			else delete node;

			node1 = visSceneNode.getChildNode( "node", ++nodeItr1 );
		}
		
		return true;
	}
};


struct DaeLibVisScenes
{
	vector< DaeVisualScene * >	visScenes;


	~DaeLibVisScenes()
	{
		for( unsigned int i = 0; i < visScenes.size(); ++i ) delete visScenes[i];
	}
	

	DaeVisualScene *findVisualScene( const string &id )
	{
		if( id == "" ) return 0x0;
		
		for( unsigned int i = 0; i < visScenes.size(); ++i )
		{
			if( visScenes[i]->id == id ) return visScenes[i];
		}

		return 0x0;
	}
	
	
	bool parse( const XMLNode &rootNode )
	{
		XMLNode node1 = rootNode.getChildNode( "library_visual_scenes" );
		if( node1.isEmpty() ) return true;

		int nodeItr2 = 0;
		XMLNode node2 = node1.getChildNode( "visual_scene", nodeItr2 );
		while( !node2.isEmpty() )
		{
			DaeVisualScene *visScene = new DaeVisualScene();

			if( visScene->parse( node2 ) ) visScenes.push_back( visScene );
			else delete visScene;

			node2 = node1.getChildNode( "visual_scene", ++nodeItr2 );
		}
		
		return true;
	}
};

#endif // _daeLibVisualScenes_H_
