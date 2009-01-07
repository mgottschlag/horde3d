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

#ifndef _daeLibNodes_H_
#define _daeLibNodes_H_

#include "utXMLParser.h"
#include <string>
#include <vector>

#include "daeLibVisualScenes.h"

struct DaeLibNodes
{
	std::vector< DaeNode * >	nodes;

	std::string	id;
	std::string	name;

	~DaeLibNodes()
	{
		for( unsigned int i = 0; i < nodes.size(); ++i ) delete nodes[i];
	}


	DaeNode *findNode( const std::string &id )
	{
		if( id == "" ) return 0x0;
		
		for( unsigned int i = 0; i < nodes.size(); ++i )
		{
			if( nodes[i]->id == id ) return nodes[i];
		}

		return 0x0;
	}

	
	bool parse( const XMLNode &rootNode )
	{
		XMLNode node1 = rootNode.getChildNode( "library_nodes" );
		if( node1.isEmpty() ) return true;

		int nodeItr2 = 0;
		XMLNode node2 = node1.getChildNode( "node", nodeItr2 );
		while( !node2.isEmpty() )
		{
			DaeNode *node = new DaeNode();
			if( node->parse( node2 ) ) nodes.push_back( node );
			else delete node;

			node2 = node1.getChildNode( "node", ++nodeItr2 );
		}
		
		return true;
	}

};
#endif
