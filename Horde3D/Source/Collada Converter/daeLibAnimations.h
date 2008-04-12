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

#ifndef _daeLibAnimations_H_
#define _daeLibAnimations_H_

#include "utXMLParser.h"
#include "daeCommon.h"
#include "utils.h"
#include <string>
#include <vector>
using namespace std;


struct DaeSampler
{
	string			id;
	DaeSource		*input;			// Time values
	DaeSource		*output;		// Transformation data
};


struct DaeChannel
{
	DaeSampler		*source;
	string			nodeId;			// Target node
	string			transSid;		// Target transformation channel
};


struct DaeAnimation
{
	string						id;
	vector< DaeSource >			sources;
	vector< DaeSampler >		samplers;
	vector< DaeChannel >		channels;
	vector< DaeAnimation * >	children;


	~DaeAnimation()
	{
		for( unsigned int i = 0; i < children.size(); ++i ) delete children[i];
	}


	DaeSource *findSource( const string &id )
	{
		if( id == "" ) return 0x0;

		for( unsigned int i = 0; i < sources.size(); ++i )
		{
			if( sources[i].id == id ) return &sources[i];
		}

		return 0x0;
	}

	
	DaeSampler *findAnimForTarget( const string &nodeId, const string &transSid )
	{
		for( unsigned int i = 0; i < channels.size(); ++i )
		{
			if( channels[i].nodeId == nodeId && channels[i].transSid == transSid )
				return channels[i].source;
		}
		
		// Parse children
		for( unsigned int i = 0; i < children.size(); ++i )
		{
			DaeSampler *sampler = children[i]->findAnimForTarget( nodeId, transSid );
			if( sampler != 0x0 ) return sampler;
		}

		return 0x0;
	}


	bool parse( const XMLNode &animNode, unsigned int &maxFrameCount )
	{
		id = animNode.getAttribute( "id", "" );
		if( id == "" ) return false;
		
		// Sources
		int nodeItr1 = 0;
		XMLNode node1 = animNode.getChildNode( "source", nodeItr1 );
		while( !node1.isEmpty() )
		{
			sources.push_back( DaeSource() );
			if( !sources[sources.size() - 1].parse( node1 ) ) sources.pop_back();

			node1 = animNode.getChildNode( "source", ++nodeItr1 );
		}
		
		// Samplers
		nodeItr1 = 0;
		node1 = animNode.getChildNode( "sampler", nodeItr1 );
		while( !node1.isEmpty() )
		{
			samplers.push_back( DaeSampler() );
			DaeSampler &sampler = samplers[samplers.size() - 1];

			sampler.id = node1.getAttribute( "id" );
			
			int nodeItr2 = 0;
			XMLNode node2 = node1.getChildNode( "input", nodeItr2 );
			while( !node2.isEmpty() )
			{
				if( strcmp( node2.getAttribute( "semantic", "" ), "INPUT" ) == 0x0 )
				{
					string id = node2.getAttribute( "source", "" );
					removeGate( id );
					sampler.input = findSource( id );
				}
				else if( strcmp( node2.getAttribute( "semantic", "" ), "OUTPUT" ) == 0x0 )
				{
					string id = node2.getAttribute( "source", "" );
					removeGate( id );
					sampler.output = findSource( id );
				}
				
				node2 = node1.getChildNode( "input", ++nodeItr2 );
			}
			
			if( sampler.input == 0x0 || sampler.output == 0x0 )
				samplers.pop_back();
			else
				maxFrameCount = max( maxFrameCount,
					(unsigned int)sampler.input->floatArray.size() / sampler.input->elemsPerEntry );

			node1 = animNode.getChildNode( "sampler", ++nodeItr1 );
		}

		// Channels
		nodeItr1 = 0;
		node1 = animNode.getChildNode( "channel", nodeItr1 );
		while( !node1.isEmpty() )
		{
			channels.push_back( DaeChannel() );
			DaeChannel &channel = channels[channels.size() - 1];

			// Parse target
			string s = node1.getAttribute( "target", "" );
			size_t pos = s.find( "/" );
			if( pos != string::npos && pos != s.length() - 1 )
			{
				channel.nodeId = s.substr( 0, pos );
				channel.transSid = s.substr( pos + 1, s.length() - pos );
				if( channel.transSid.find( "(" ) != string::npos ||
					channel.transSid.find( "." ) != string::npos )
				{
					log( "Warning: Animation for " + channel.nodeId + " has unsupported address syntax"  );
				}
			}
			
			// Find source
			s = node1.getAttribute( "source", "" );
			removeGate( s );
			if( s != "" )
			{
				for( unsigned int i = 0; i < samplers.size(); ++i )
				{
					if( samplers[i].id == s )
					{
						channel.source = &samplers[i];
						break;
					}
				}
			}

			if( channel.nodeId == "" || channel.transSid == "" || channel.source == 0x0 )
			{
				log( "Warning: Missing channel attributes or sampler not found" );
				channels.pop_back();
			}

			node1 = animNode.getChildNode( "channel", ++nodeItr1 );
		}
		
		// Parse children
		nodeItr1 = 0;
		node1 = animNode.getChildNode( "animation", nodeItr1 );
		while( !node1.isEmpty() )
		{
			DaeAnimation *anim = new DaeAnimation();
			if( anim->parse( node1, maxFrameCount ) ) children.push_back( anim );
			else delete anim;

			node1 = animNode.getChildNode( "animation", ++nodeItr1 );
		}
		
		return true;
	}
};


struct DaeLibAnimations
{
	vector< DaeAnimation * >	animations;
	unsigned int				maxFrameCount;
	

	~DaeLibAnimations()
	{
		for( unsigned int i = 0; i < animations.size(); ++i ) delete animations[i];
	}
	

	DaeSampler *findAnimForTarget( const string &nodeId, string const &transSid )
	{
		for( unsigned int i = 0; i < animations.size(); ++i )
		{
			DaeSampler *sampler = animations[i]->findAnimForTarget( nodeId, transSid );
			if( sampler != 0x0 ) return sampler;
		}

		return 0x0;
	}

	
	bool parse( const XMLNode &rootNode )
	{
		maxFrameCount = 0;
		
		XMLNode node1 = rootNode.getChildNode( "library_animations" );
		if( node1.isEmpty() ) return true;

		int nodeItr2 = 0;
		XMLNode node2 = node1.getChildNode( "animation", nodeItr2 );
		while( !node2.isEmpty() )
		{
			DaeAnimation *anim = new DaeAnimation();
			if( anim->parse( node2, maxFrameCount ) ) animations.push_back( anim );
			else delete anim;

			node2 = node1.getChildNode( "animation", ++nodeItr2 );
		}
		
		return true;
	}
};

#endif // _daeLibAnimations_H_
