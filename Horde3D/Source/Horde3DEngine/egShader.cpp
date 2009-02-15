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
	_flagMask = 0;
	_code.clear();
}


void CodeResource::release()
{
	for( uint32 i = 0; i < _includes.size(); ++i )
	{
		_includes[i].first = 0x0;
	}
	_includes.clear();
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

		// Check for flags
		if( !lineComment && !blockComment && pData < eof - 4 )
		{
			if( *pData == '_' && *(pData+1) == 'F' && *(pData+4) == '_' &&
			    *(pData+2) >= 48 && *(pData+2) <= 57 && *(pData+3) >= 48 && *(pData+3) <= 57 )
			{
				// Set flag
				uint32 num = (*(pData+2) - 48) * 10 + (*(pData+3) - 48);
				_flagMask |= 1 << (num - 1);
				
				for( uint32 i = 0; i < 5; ++i ) *pCode++ = *pData++;
				
				// Ignore rest of name
				while( pData < eof && *pData != ' ' && *pData != '\t' && *pData != '\n' && *pData != '\r' )
					++pData;
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


bool CodeResource::tryLinking( uint32 *flagMask )
{
	if( !_loaded ) return false;
	if( flagMask != 0x0 ) *flagMask |= _flagMask;
	
	for( uint32 i = 0; i < _includes.size(); ++i )
	{
		if( !_includes[i].first->tryLinking( flagMask ) ) return false;
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
		std::string depCode = _includes[i].first->assembleCode();
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
				ShaderContext &con = shaderRes->getContexts()[j];

				if( con.vertCode->hasDependency( this ) || con.fragCode->hasDependency( this ) )
				{
					con.compiled = false;
				}
			}
			
			// Recompile shaders
			shaderRes->compileContexts();
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
		for( uint32 j = 0; j < _contexts[i].shaderCombs.size(); ++j )
		{
			Modules::renderer().unloadShader( _contexts[i].shaderCombs[j].shaderObject );
		}
	}

	_contexts.clear();
	_samplers.clear();
	_uniforms.clear();
	//_preLoadList.clear();
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


bool ShaderResource::parseFXSection( const char *data )
{
	// Parse FX section
	XMLResults res;
	XMLNode rootNode = XMLNode::parseString( data, "Shader", &res );
	if( res.error != eXMLErrorNone )
	{
		return raiseError( XMLNode::getError( res.error ), res.nLine );
	}

	// Parse contexts
	int nodeItr1 = 0;
	XMLNode node1 = rootNode.getChildNode( "Context", nodeItr1 );
	while( !node1.isEmpty() )
	{
		if( node1.getAttribute( "id" ) == 0x0 ) return raiseError( "Missing Context attribute 'id'" );
		
		ShaderContext context;

		context.id = node1.getAttribute( "id" );
		
		// Config
		XMLNode node2 = node1.getChildNode( "RenderConfig" );
		if( !node2.isEmpty() )
		{
			// Depth mask
			if( _stricmp( node2.getAttribute( "writeDepth", "true" ), "false" ) == 0 ||
				_stricmp( node2.getAttribute( "writeDepth", "1" ), "0" ) == 0 )
				context.writeDepth = false;
			else
				context.writeDepth = true;

			// Blending
			if( _stricmp( node2.getAttribute( "blendMode", "REPLACE" ), "BLEND" ) == 0 )
				context.blendMode = BlendModes::Blend;
			else if( _stricmp( node2.getAttribute( "blendMode", "REPLACE" ), "ADD" ) == 0 )
				context.blendMode = BlendModes::Add;
			else if( _stricmp( node2.getAttribute( "blendMode", "REPLACE" ), "ADD_BLENDED" ) == 0 )
				context.blendMode = BlendModes::AddBlended;
			else if( _stricmp( node2.getAttribute( "blendMode", "REPLACE" ), "MULT" ) == 0 )
				context.blendMode = BlendModes::Mult;
			else
				context.blendMode = BlendModes::Replace;

			// Depth test
			if( _stricmp( node2.getAttribute( "depthTest", "LESS_EQUAL" ), "ALWAYS" ) == 0 )
				context.depthTest = TestModes::Always;
			else if( _stricmp( node2.getAttribute( "depthTest", "LESS_EQUAL" ), "EQUAL" ) == 0 )
				context.depthTest = TestModes::Equal;
			else if( _stricmp( node2.getAttribute( "depthTest", "LESS_EQUAL" ), "LESS" ) == 0 )
				context.depthTest = TestModes::Less;
			else if( _stricmp( node2.getAttribute( "depthTest", "LESS_EQUAL" ), "GREATER" ) == 0 )
				context.depthTest = TestModes::Greater;
			else if( _stricmp( node2.getAttribute( "depthTest", "LESS_EQUAL" ), "GREATER_EQUAL" ) == 0 )
				context.depthTest = TestModes::GreaterEqual;
			else
				context.depthTest = TestModes::LessEqual;

			// Alpha test
			if( _stricmp( node2.getAttribute( "alphaTest", "ALWAYS" ), "EQUAL" ) == 0 )
				context.alphaTest = TestModes::Equal;
			else if( _stricmp( node2.getAttribute( "alphaTest", "ALWAYS" ), "LESS" ) == 0 )
				context.alphaTest = TestModes::Less;
			else if( _stricmp( node2.getAttribute( "alphaTest", "ALWAYS" ), "LESS_EQUAL" ) == 0 )
				context.alphaTest = TestModes::LessEqual;
			else if( _stricmp( node2.getAttribute( "alphaTest", "ALWAYS" ), "GREATER" ) == 0 )
				context.alphaTest = TestModes::Greater;
			else if( _stricmp( node2.getAttribute( "alphaTest", "ALWAYS" ), "GREATER_EQUAL" ) == 0 )
				context.alphaTest = TestModes::GreaterEqual;
			else
				context.alphaTest = TestModes::Always;
			
			context.alphaRef = (float)atof( node2.getAttribute( "alphaRef", "0" ) );

			// Alpha-to-coverage
			if( _stricmp( node2.getAttribute( "alphaToCoverage", "false" ), "true" ) == 0 ||
				_stricmp( node2.getAttribute( "alphaToCoverage", "0" ), "1" ) == 0 )
				context.alphaToCoverage = true;
			else
				context.alphaToCoverage = false;
		}
		
		// Shaders
		node2 = node1.getChildNode( "Shaders" );
		if( node2.isEmpty() ) return raiseError( "Missing Shaders node in Context '" + context.id + "'" );

		_tmpCode0 = _name + ":" + node2.getAttribute( "vertex", "" );
		_tmpCode1 = _name + ":" + node2.getAttribute( "fragment", "" );
		
		context.vertCode = (CodeResource *)Modules::resMan().findResource( ResourceTypes::Code, _tmpCode0.c_str() );
		if( context.vertCode == 0x0 )
			return raiseError( "Context '" + context.id + "' references undefined vertex shader code section" );
		context.fragCode = (CodeResource *)Modules::resMan().findResource( ResourceTypes::Code, _tmpCode1.c_str() );
		if( context.fragCode == 0x0 )
			return raiseError( "Context '" + context.id + "' references undefined fragment shader code section" );

		_contexts.push_back( context );
		
		node1 = rootNode.getChildNode( "Context", ++nodeItr1 );
	}

	// Parse samplers
	bool unitFree[12] = {true, true, true, true, true, true, true, true, true, true, true, true}; 
	
	nodeItr1 = 0;
	node1 = rootNode.getChildNode( "Sampler", nodeItr1 );
	while( !node1.isEmpty() )
	{
		if( node1.getAttribute( "id" ) == 0x0 ) return raiseError( "Missing Sampler attribute 'id'" );
		
		ShaderSampler sampler;

		sampler.id = node1.getAttribute( "id" );
		sampler.texUnit = atoi( node1.getAttribute( "texUnit", "-1" ) );
		if( sampler.texUnit > 11 ) return raiseError( "texUnit exceeds limit" );
		if( sampler.texUnit >= 0 ) unitFree[sampler.texUnit] = false;

		// Sampler states
		XMLNode node2 = node1.getChildNode( "StageConfig" );
		if( !node2.isEmpty() )
		{
			// Address mode
			if( _stricmp( node2.getAttribute( "addressMode", "WRAP" ), "CLAMP" ) == 0 )
				sampler.addressMode = TexAddressModes::Clamp;
			else
				sampler.addressMode = TexAddressModes::Wrap;
			
			// Filtering
			if( _stricmp( node2.getAttribute( "filtering", "TRILINEAR" ), "NONE" ) == 0 )
				sampler.filterMode = TexFilterModes::None;
			else if( _stricmp( node2.getAttribute( "filtering", "TRILINEAR" ), "BILINEAR" ) == 0 )
				sampler.filterMode = TexFilterModes::Bilinear;
			else
				sampler.filterMode = TexFilterModes::Trilinear;

			// Max anisotropy
			sampler.maxAnisotropy = atoi( node2.getAttribute( "maxAnisotropy", "8" ) );
		}

		_samplers.push_back( sampler );

		node1 = rootNode.getChildNode( "Sampler", ++nodeItr1 );
	}

	// Parse uniforms
	nodeItr1 = 0;
	node1 = rootNode.getChildNode( "Uniform", nodeItr1 );
	while( !node1.isEmpty() )
	{
		if( node1.getAttribute( "id" ) == 0x0 ) return raiseError( "Missing Uniform attribute 'id'" );
		
		ShaderUniform uniform;

		uniform.id = node1.getAttribute( "id" );
		uniform.defValues[0] = (float)atof( node1.getAttribute( "a", "0.0" ) );
		uniform.defValues[1] = (float)atof( node1.getAttribute( "b", "0.0" ) );
		uniform.defValues[2] = (float)atof( node1.getAttribute( "c", "0.0" ) );
		uniform.defValues[3] = (float)atof( node1.getAttribute( "d", "0.0" ) );

		_uniforms.push_back( uniform );
		
		node1 = rootNode.getChildNode( "Uniform", ++nodeItr1 );
	}

	// Automatic texture unit assignment
	for( uint32 i = 0; i < _samplers.size(); ++i )
	{
		if( _samplers[i].texUnit < 0 )
		{	
			for( uint32 j = 0; j < 12; ++j )
			{
				if( unitFree[j] )
				{
					_samplers[i].texUnit = j;
					unitFree[j] = false;
					break;
				}
			}
			if( _samplers[i].texUnit < 0 )
				return raiseError( "Automatic texture unit assignment: No free unit found" );
		}
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

	if( _tmpCode1.empty() ) return raiseError( "Missing FX section" );
	if( !parseFXSection( _tmpCode1.c_str() ) ) return false;

	compileContexts();
	
	return true;
}


void ShaderResource::preLoadCombination( uint32 combMask )
{
	if( !_loaded )
	{
		_preLoadList.insert( combMask );
	}
	else
	{
		for( uint32 i = 0; i < _contexts.size(); ++i )
		{
			if( getCombination( _contexts[i], combMask ) == 0x0 )
				_preLoadList.insert( combMask );
		}
	}
}


void ShaderResource::compileCombination( ShaderContext &context, ShaderCombination &sc )
{
	uint32 combMask = sc.combMask;
	
	// Add preamble
	_tmpCode0 = _vertPreamble;
	_tmpCode1 = _fragPreamble;

	// Insert defines for flags
	if( combMask != 0 )
	{
		_tmpCode0 += "\r\n// ---- Flags ----\r\n";
		_tmpCode1 += "\r\n// ---- Flags ----\r\n";
		for( uint32 i = 1; i <= 32; ++i )
		{
			if( combMask & (1 << (i-1)) )
			{
				_tmpCode0 += "#define _F";
				_tmpCode0 += (char)(48 + i / 10);
				_tmpCode0 += (char)(48 + i % 10);
				_tmpCode0 += "_\r\n";
				
				_tmpCode1 += "#define _F";
				_tmpCode1 += (char)(48 + i / 10);
				_tmpCode1 += (char)(48 + i % 10);
				_tmpCode1 += "_\r\n";
			}
		}
		_tmpCode0 += "// ---------------\r\n";
		_tmpCode1 += "// ---------------\r\n";
	}

	// Add actual shader code
	_tmpCode0 += context.vertCode->assembleCode();
	_tmpCode1 += context.fragCode->assembleCode();

	
	Modules::log().writeInfo( "---- C O M P I L I N G  . S H A D E R . %s@%s[%i] ----",
		_name.c_str(), context.id.c_str(), sc.combMask );
	
	// Unload shader if necessary
	if( sc.shaderObject != 0 )
	{
		Modules::renderer().unloadShader( sc.shaderObject );
		sc.shaderObject = 0;
	}
	
	// Compile shader
	if( !Modules::renderer().uploadShader( _tmpCode0.c_str(), _tmpCode1.c_str(), sc ) )
	{
		Modules::log().writeError( "Shader resource '%s': Failed to compile shader context '%s' (comb %i)",
			_name.c_str(), context.id.c_str(), sc.combMask );

		if( Modules::config().dumpFailedShaders )
		{
			std::ofstream out0( "shdDumpVS.txt", ios::binary ), out1( "shdDumpFS.txt", ios::binary );
			if( out0.good() ) out0 << _tmpCode0;
			if( out1.good() ) out1 << _tmpCode1;
			out0.close();
			out1.close();
		}
	}

	// Find samplers in compiled shader
	sc.customSamplers.reserve( _samplers.size() );
	for( uint32 i = 0; i < _samplers.size(); ++i )
	{
		sc.customSamplers.push_back(
			Modules::renderer().getShaderVar( sc.shaderObject, _samplers[i].id.c_str() ) );
		
		// Set texture unit
		Modules::renderer().setShaderVar1i( sc.shaderObject, _samplers[i].id.c_str(), _samplers[i].texUnit );
	}
	
	// Find uniforms in compiled shader
	sc.customUniforms.reserve( _uniforms.size() );
	for( uint32 i = 0; i < _uniforms.size(); ++i )
	{
		sc.customUniforms.push_back(
			Modules::renderer().getShaderVar( sc.shaderObject, _uniforms[i].id.c_str() ) );
	}

	// Output shader log
	if( Modules::renderer().getShaderLog() != "" )
		Modules::log().writeInfo( "Shader resource '%s': ShaderLog: %s", _name.c_str(), Modules::renderer().getShaderLog().c_str() );
}


void ShaderResource::compileContexts()
{
	for( uint32 i = 0; i < _contexts.size(); ++i )
	{
		ShaderContext &context = _contexts[i];

		if( !context.compiled )
		{
			context.flagMask = 0;
			if( !context.vertCode->tryLinking( &context.flagMask ) ||
				!context.fragCode->tryLinking( &context.flagMask ) )
			{
				continue;
			}
			
			// Add preloaded combinations
			for( std::set< uint32 >::iterator itr = _preLoadList.begin(); itr != _preLoadList.end(); ++itr )
			{
				uint32 combMask = *itr & context.flagMask;
				
				// Check if combination already exists
				bool found = false;
				for( size_t j = 0; j < _contexts[i].shaderCombs.size(); ++j )
				{
					if( _contexts[i].shaderCombs[j].combMask == combMask )
					{
						found = true;
						break;
					}
				}

				if( !found )
				{	
					_contexts[i].shaderCombs.push_back( ShaderCombination() );
					_contexts[i].shaderCombs.back().combMask = combMask;
				}
			}
			
			for( size_t j = 0; j < _contexts[i].shaderCombs.size(); ++j )
			{
				compileCombination( _contexts[i], _contexts[i].shaderCombs[j] );
			}

			context.compiled = true;
		}
	}
}


ShaderCombination *ShaderResource::getCombination( ShaderContext &context, uint32 combMask )
{
	if( !context.compiled ) return 0x0;
	
	// Kill combination bits that are not used by the context
	combMask &= context.flagMask;
	
	// Try to find combination
	std::vector< ShaderCombination > &combs = context.shaderCombs;
	for( size_t i = 0, s = combs.size(); i < s; ++i )
	{
		if( combs[i].combMask == combMask ) return &combs[i];
	}

	// Add combination
	combs.push_back( ShaderCombination() );
	combs.back().combMask = combMask;
	compileCombination( context, combs.back() );

	return &combs.back();
}


uint32 ShaderResource::calcCombMask( const std::vector< std::string > &flags )
{	
	uint32 combMask = 0;
	
	for( size_t i = 0, s = flags.size(); i < s; ++i )
	{
		const string &flag = flags[i];
		
		// Check format: _F<digit><digit>_
		if( flag.length() < 5 ) continue;
		if( flag[0] != '_' || flag[1] != 'F' || flag[4] != '_' ||
		    flag[2] < 48 || flag[2] > 57 || flag[3] < 48 || flag[3] > 57 ) continue;
		
		uint32 num = (flag[2] - 48) * 10 + (flag[3] - 48);
		combMask |= 1 << (num - 1);
	}
	
	return combMask;
}
