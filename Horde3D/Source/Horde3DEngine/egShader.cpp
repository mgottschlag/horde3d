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

#include "egShader.h"
#include "egModules.h"
#include "utXMLParser.h"
#include "utPlatform.h"
#include <sstream>
#include <fstream>

#include "utDebug.h"

using namespace std;

// *************************************************************************************************
// CodeResource
// *************************************************************************************************

CodeResource::CodeResource( const string &name, int flags ) :
	Resource( ResourceTypes::Code, name, flags )
{
	initDefault();
}


CodeResource::~CodeResource()
{
	release();
}


Resource *CodeResource::clone()
{
	CodeResource *res = new CodeResource( "", _flags );

	*res = *this;
	
	return res;
}


void CodeResource::initDefault()
{
	_code.clear();
}


void CodeResource::release()
{
	for( uint32 i = 0; i < _includes.size(); ++i )
	{
		_includes[i].first = 0x0;
	}
}


bool CodeResource::raiseError( const std::string &msg )
{
	// Reset
	release();
	initDefault();
	
	Modules::log().writeError( "Code resource '%s': %s", _name.c_str(), msg.c_str() );

	return false;
}


bool CodeResource::load( const char *data, int size )
{
	if( !Resource::load( data, size ) ) return false;

	char *code = new char[size+1];
	char *pCode = code;
	const char *pData = data;
	const char *eof = data + size;
	
	bool lineComment = false, blockComment = false;
	
	// Parse code
	while( pData < eof )
	{
		// Check for begin of comment
		if( pData < eof - 1 && !lineComment && !blockComment )
		{
			if( *pData == '/' && *(pData+1) == '/' )
				lineComment = true;
			else if( *pData == '/' &&  *(pData+1) == '*' )
				blockComment = true;
		}

		// Check for end of comment
		if( lineComment && (*pData == '\n' || *pData == '\r') )
			lineComment = false;
		else if( blockComment && pData < eof - 1 && *pData == '*' && *(pData+1) == '/' )
			blockComment = false;

		// Check for includes
		if( !lineComment && !blockComment && pData < eof - 7 )
		{
			if( *pData == '#' && *(pData+1) == 'i' && *(pData+2) == 'n' && *(pData+3) == 'c' &&
			    *(pData+4) == 'l' && *(pData+5) == 'u' && *(pData+6) == 'd' && *(pData+7) == 'e' )
			{
				pData += 6;
				
				// Parse resource name
				const char *nameBegin = 0x0, *nameEnd = 0x0;
				
				while( ++pData < eof )
				{
					if( *pData == '"' )
					{
						if( nameBegin == 0x0 )
							nameBegin = pData+1;
						else
							nameEnd = pData;
					}
					else if( *pData == '\n' || *pData == '\r' ) break;
				}

				if( nameBegin != 0x0 && nameEnd != 0x0 )
				{
					std::string resName( nameBegin, nameEnd );
					
					ResHandle res =  Modules::resMan().addResource(
						ResourceTypes::Code, resName, 0, false );
					CodeResource *codeRes = (CodeResource *)Modules::resMan().resolveResHandle( res );
					_includes.push_back( std::pair< PCodeResource, size_t >( codeRes, pCode - code ) );
				}
				else
				{
					delete[] code;
					return raiseError( "Invalid #include syntax" );
				}
			}
		}

		*pCode++ = *pData++;
	}

	*pCode = '\0';
	_code = code;
	delete[] code;

	// Compile shaders that require this code block
	updateShaders();

	return true;
}


bool CodeResource::hasDependency( CodeResource *codeRes )
{
	// Note: There is no check for cycles
	
	if( codeRes == this ) return true;
	
	for( uint32 i = 0; i < _includes.size(); ++i )
	{
		if( _includes[i].first->hasDependency( codeRes ) ) return true;
	}
	
	return false;
}


bool CodeResource::isComplete()
{
	if( !_loaded ) return false;
	
	for( uint32 i = 0; i < _includes.size(); ++i )
	{
		if( !_includes[i].first->isComplete() ) return false;
	}

	return true;
}


std::string CodeResource::assembleCode()
{
	if( !_loaded ) return "";

	std::string finalCode = _code;
	uint32 offset = 0;
	
	for( uint32 i = 0; i < _includes.size(); ++i )
	{
		std::string &depCode = _includes[i].first->assembleCode();
		finalCode.insert( _includes[i].second + offset, depCode );
		offset += (uint32)depCode.length();
	}

	return finalCode;
}


void CodeResource::updateShaders()
{
	for( uint32 i = 0; i < Modules::resMan().getResources().size(); ++i )
	{
		Resource *res = Modules::resMan().getResources()[i];

		if( res != 0x0 && res->getType() == ResourceTypes::Shader )
		{
			ShaderResource *shaderRes = (ShaderResource *)res;
			
			// Mark shaders using this code as uncompiled
			for( uint32 j = 0; j < shaderRes->getContexts().size(); ++j )
			{
				ShaderContext &sc = shaderRes->getContexts()[j];

				if( sc.vertCode->hasDependency( this ) || sc.fragCode->hasDependency( this ) )
				{
					sc.compiled = false;
				}
			}
			
			// Recompile shaders
			shaderRes->compileShaders();
		}
	}
}


// *************************************************************************************************
// ShaderResource
// *************************************************************************************************

string ShaderResource::_vertPreamble = "";
string ShaderResource::_fragPreamble = "";
string ShaderResource::_tmpCode0 = "";
string ShaderResource::_tmpCode1 = "";


ShaderResource::ShaderResource( const string &name, int flags ) :
	Resource( ResourceTypes::Shader, name, flags )
{
	initDefault();
}


ShaderResource::~ShaderResource()
{
	release();
}


void ShaderResource::initDefault()
{
}


void ShaderResource::release()
{
	for( uint32 i = 0; i < _contexts.size(); ++i )
	{
		Modules::renderer().unloadShader( _contexts[i].shaderObject );
	}

	_contexts.clear();
}


bool ShaderResource::raiseError( const string &msg, int line )
{
	// Reset
	release();
	initDefault();

	if( line < 0 )
		Modules::log().writeError( "Shader resource '%s': %s", _name.c_str(), msg.c_str() );
	else
		Modules::log().writeError( "Shader resource '%s' in line %i: %s", _name.c_str(), line, msg.c_str() );
	
	return false;
}


bool ShaderResource::parseXMLCode( XMLNode &node, std::string &code )
{
	code = "";
	
	int nodeItr1 = 0;
	XMLNode node1 = node.getChildNode( nodeItr1 );
	while( !node1.isEmpty() && node1.getName() != 0x0 )
	{
		if( strcmp( node1.getName(), "DefCode" ) == 0 )
		{
			// Find CDATA
			for( int i = 0; i < node1.nClear(); ++i )
			{
				if( strcmp( node1.getClear( i ).lpszOpenTag, "<![CDATA[" ) == 0 )
				{
					code += node1.getClear().lpszValue;
					break;
				}
			}
		}
		else if( strcmp( node1.getName(), "InsCode" ) == 0 )
		{
			if( node1.getAttribute( "code" ) == 0x0 ) return false;
		
			code += "\r\n#include \"";
			code += node1.getAttribute( "code" );
			code += "\"\r\n";
		}

		node1 = node.getChildNode( ++nodeItr1 );
	}

	return true;
}


bool ShaderResource::parseFXSection( const char *data, bool oldFormat )
{
	// Parse FX
	XMLResults res;
	XMLNode rootNode = XMLNode::parseString( data, "Shader", &res );
	if( res.error != eXMLErrorNone )
	{
		return raiseError( XMLNode::getError( res.error ), res.nLine );
	}

	if( oldFormat )
		Modules::log().writeWarning( "Shader resource '%s': Deprecated old syntax, please consider converting to new format", _name.c_str() );
	
	int nodeItr1 = 0;
	XMLNode node1 = rootNode.getChildNode( "Context", nodeItr1 );
	while( !node1.isEmpty() )
	{
		if( node1.getAttribute( "id" ) == 0x0 ) return raiseError( "Missing Context attribute 'id'" );
		
		ShaderContext sc;

		sc.id = node1.getAttribute( "id" );
		
		// Config
		XMLNode node2 = node1.getChildNode( "RenderConfig" );
		if( !node2.isEmpty() )
		{
			if( _stricmp( node2.getAttribute( "writeDepth", "true" ), "false" ) == 0 ||
				_stricmp( node2.getAttribute( "writeDepth", "1" ), "0" ) == 0 )
				sc.writeDepth = false;
			else
				sc.writeDepth = true;

			if( _stricmp( node2.getAttribute( "blendMode", "REPLACE" ), "BLEND" ) == 0 )
				sc.blendMode = BlendModes::Blend;
			else if( _stricmp( node2.getAttribute( "blendMode", "REPLACE" ), "ADD" ) == 0 )
				sc.blendMode = BlendModes::Add;
			else if( _stricmp( node2.getAttribute( "blendMode", "REPLACE" ), "ADD_BLENDED" ) == 0 )
				sc.blendMode = BlendModes::AddBlended;
			else if( _stricmp( node2.getAttribute( "blendMode", "REPLACE" ), "MULT" ) == 0 )
				sc.blendMode = BlendModes::Mult;
			else
				sc.blendMode = BlendModes::Replace;
		}
		
		if( oldFormat )
		{
			// Create vertex shader code resource
			node2 = node1.getChildNode( "VertexShader" );
			if( node2.isEmpty() ) return raiseError( "Missing VertexShader node in Context '" + sc.id + "'" );
			if( !parseXMLCode( node2, _tmpCode0 ) ) return raiseError( "Error in VertexShader node of Context '" + sc.id + "'" );
			
			ResHandle res = Modules::resMan().addResource(
				ResourceTypes::Code, _name + ":VS_" + sc.id, 0, false );
			sc.vertCode = (CodeResource *)Modules::resMan().resolveResHandle( res );
			sc.vertCode->load( _tmpCode0.c_str(), (uint32)_tmpCode0.length() );
			
			// Create fragment shader code resource
			node2 = node1.getChildNode( "FragmentShader" );
			if( node2.isEmpty() ) return raiseError( "Missing FragmentShader node in Context '" + sc.id + "'" );
			if( !parseXMLCode( node2, _tmpCode0 ) ) return raiseError( "Error in FragmentShader node of Context '" + sc.id + "'" );

			res = Modules::resMan().addResource(
				ResourceTypes::Code, _name + ":FS_" + sc.id, 0, false );
			sc.fragCode = (CodeResource *)Modules::resMan().resolveResHandle( res );
			sc.fragCode->load( _tmpCode0.c_str(), (uint32)_tmpCode0.length() );
		}
		else
		{
			node2 = node1.getChildNode( "Shaders" );
			if( node2.isEmpty() ) return raiseError( "Missing Shaders node in Context '" + sc.id + "'" );

			_tmpCode0 = _name + ":" + node2.getAttribute( "vertex", "" );
			_tmpCode1 = _name + ":" + node2.getAttribute( "fragment", "" );
			
			sc.vertCode = (CodeResource *)Modules::resMan().findResource( ResourceTypes::Code, _tmpCode0.c_str() );
			if( sc.vertCode == 0x0 )
				return raiseError( "Context '" + sc.id + "' references undefined vertex shader code section" );
			sc.fragCode = (CodeResource *)Modules::resMan().findResource( ResourceTypes::Code, _tmpCode1.c_str() );
			if( sc.fragCode == 0x0 )
				return raiseError( "Context '" + sc.id + "' references undefined fragment shader code section" );
		}

		_contexts.push_back( sc );
		
		node1 = rootNode.getChildNode( "Context", ++nodeItr1 );
	}

	return true;
}


bool ShaderResource::load( const char *data, int size )
{
	if( !Resource::load( data, size ) ) return false;
	if( data[size - 1] != '\0' )
	{	
		return raiseError( "Data block not NULL-terminated" );
	}
	
	// Parse sections
	const char *pData = data;
	_tmpCode1 = "";
	
	while( *pData != '\0' )
	{
		if( *pData++ == '[' && *pData++ == '[' )
		{
			// Parse section name
			const char *sectionNameStart = pData;
			while( *pData != ']' && *pData != '\n' && *pData != '\r' ) ++pData;
			const char *sectionNameEnd = pData++;

			// Check for correct closing of name
			if( *pData++ != ']' ) return raiseError( "Error in section name" );
			
			// Parse content
			const char *sectionContentStart = pData;
			while( *pData != '\0' && *pData != '[' && *(pData+1) != '[' ) ++pData;
			const char *sectionContentEnd = pData;
			

			if( sectionNameEnd - sectionNameStart == 2 &&
			    *sectionNameStart == 'F' && *(sectionNameStart+1) == 'X' )
			{
				// Parse FX section
				_tmpCode1 = "<Shader>";
				_tmpCode1.append( sectionContentStart, sectionContentEnd );
				_tmpCode1 += "</Shader>";
			}
			else
			{
				// Add section as code resource
				_tmpCode0.assign( sectionNameStart, sectionNameEnd );
				ResHandle res = Modules::resMan().addResource(
					ResourceTypes::Code, _name + ":" + _tmpCode0, 0, false );
				CodeResource *codeRes = (CodeResource *)Modules::resMan().resolveResHandle( res );
				
				_tmpCode0.assign( sectionContentStart, sectionContentEnd );
				codeRes->load( _tmpCode0.c_str(), (uint32)_tmpCode0.length() );
			}
		}
	}
	
	bool result;

	if( !_tmpCode1.empty() )
		result = parseFXSection( _tmpCode1.c_str(), false );
	else
		result = parseFXSection( data, true );

	if( !result ) return false;

	compileShaders();
	
	return true;
}


void ShaderResource::compileShaders()
{
	for( uint32 i = 0; i < _contexts.size(); ++i )
	{
		ShaderContext &sc = _contexts[i];

		if( !sc.compiled )
		{
			if( !sc.vertCode->isComplete() || !sc.fragCode->isComplete() ) continue;

			_tmpCode0 = _vertPreamble;
			_tmpCode0 += "\r\n";
			_tmpCode0 += sc.vertCode->assembleCode();
			_tmpCode1 = _vertPreamble;
			_tmpCode1 += "\r\n";
			_tmpCode1 += sc.fragCode->assembleCode();
			
			// Compile shader
			Modules::log().writeInfo( "Shader resource '%s': Compiling shader context '%s'", _name.c_str(), sc.id.c_str() );
			if( sc.shaderObject != 0 )
			{
				Modules::renderer().unloadShader( sc.shaderObject );
				sc.shaderObject = 0;
			}
			
			if( !Modules::renderer().uploadShader( _tmpCode0.c_str(), _tmpCode1.c_str(), sc ) )
			{
				Modules::log().writeError( "Shader resource '%s': Failed to compile shader context '%s'", _name.c_str(), sc.id.c_str() );

				if( Modules::config().dumpFailedShaders )
				{
					std::ofstream out0( "shdDumpVS.txt", ios::binary ), out1( "shdDumpFS.txt", ios::binary );
					if( out0.good() ) out0 << _tmpCode0;
					if( out1.good() ) out1 << _tmpCode1;
					out0.close();
					out1.close();
				}
			}

			if( Modules::renderer().getShaderLog() != "" )
				Modules::log().writeInfo( "Shader resource '%s': ShaderLog: %s", _name.c_str(), Modules::renderer().getShaderLog().c_str() );

			sc.compiled = true;
		}
	}	
}
