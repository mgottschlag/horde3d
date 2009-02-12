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

#ifndef _egShader_H_
#define _egShader_H_

#include "egPrerequisites.h"
#include "egResource.h"
#include <set>

struct XMLNode;


// =================================================================================================
// Code Resource
// =================================================================================================

class CodeResource;
typedef SmartResPtr< CodeResource > PCodeResource;

class CodeResource : public Resource
{
private:
	
	uint32                                             _flagMask;
	std::string                                        _code;
	std::vector< std::pair< PCodeResource, size_t > >  _includes;	// Pair: Included res and location in _code

	bool raiseError( const std::string &msg );
	void updateShaders();

public:
	
	static Resource *factoryFunc( const std::string &name, int flags )
		{ return new CodeResource( name, flags ); }
	
	CodeResource( const std::string &name, int flags );
	~CodeResource();
	Resource *clone();
	
	void initDefault();
	void release();
	bool load( const char *data, int size );

	bool hasDependency( CodeResource *codeRes );
	bool tryLinking( uint32 *flagMask );
	std::string assembleCode();

	bool isLoaded() { return _loaded; }
	const std::string &getCode() { return _code; }

	friend class Renderer;
};


// =================================================================================================
// Shader Resource
// =================================================================================================

struct BlendModes
{
	enum List
	{
		Replace,
		Blend,
		Add,
		AddBlended,
		Mult
	};
};

struct TestModes
{
	enum List
	{
		Always,  // Same as disabled
		Equal,
		Less,
		LessEqual,
		Greater,
		GreaterEqual
	};
};


struct ShaderCombination
{
	uint32                          combMask;
	
	uint32                          shaderObject;
	uint32                          lastUpdateStamp;

	// Engine uniform and attribute locations
	int                             uni_frameBufSize;
	int                             uni_texs[12];
	int                             uni_worldMat, uni_worldNormalMat;
	int                             uni_viewer;
	int                             uni_lightPos, uni_lightDir, uni_lightColor, uni_lightCosCutoff;
	int                             uni_shadowSplitDists, uni_shadowMats;
	int                             uni_shadowMapSize, uni_shadowBias;
	int                             uni_skinMatRows;
	int                             uni_parCorners;
	int                             uni_parPosArray, uni_parSizeAndRotArray, uni_parColorArray;
	int                             uni_olayColor;
	int                             attrib_normal, attrib_tangent, attrib_bitangent;
	int                             attrib_joints, attrib_weights;
	int                             attrib_texCoords0, attrib_texCoords1;

	std::vector< int >              customSamplers;
	std::vector< int >              customUniforms;


	ShaderCombination() :
		combMask( 0 ), shaderObject( 0 ), lastUpdateStamp( 0 )
	{
	}
};


struct ShaderContext
{
	std::string                       id;
	uint32                            flagMask;
	
	// RenderConfig
	BlendModes::List                  blendMode;
	TestModes::List                   depthTest;
	TestModes::List                   alphaTest;
	float                             alphaRef;
	bool                              writeDepth;
	bool                              alphaToCoverage;
	
	// Shaders
	std::vector< ShaderCombination >  shaderCombs;
	PCodeResource                     vertCode, fragCode;
	bool                              compiled;


	ShaderContext() :
		compiled( false ), writeDepth( true ), blendMode( BlendModes::Replace ),
		depthTest( TestModes::LessEqual ), alphaTest( TestModes::Always ),
		alphaRef( 0.0f ), alphaToCoverage( false )
	{
	}

	~ShaderContext()
	{
		vertCode = 0x0;
		fragCode = 0x0;
	}
};

// =================================================================================================

struct ShaderSampler
{
	std::string  id;
	int          texUnit;
};

struct ShaderUniform
{
	std::string  id;
	float        defValues[4];
};


class ShaderResource : public Resource
{
private:
	
	static std::string            _vertPreamble, _fragPreamble;
	static std::string            _tmpCode0, _tmpCode1;
	
	std::vector< ShaderContext >  _contexts;
	std::vector< ShaderSampler >  _samplers;
	std::vector< ShaderUniform >  _uniforms;
	std::set< uint32 >            _preLoadList;

	bool raiseError( const std::string &msg, int line = -1 );
	bool parseXMLCode( XMLNode &node, std::string &code );
	bool parseFXSection( const char *data );
	void compileCombination( ShaderContext &context, ShaderCombination &sc );

public:
	
	static Resource *factoryFunc( const std::string &name, int flags )
		{ return new ShaderResource( name, flags ); }

	static void setPreambles( const std::string &vertPreamble, const std::string &fragPreamble )
		{ _vertPreamble = vertPreamble; _fragPreamble = fragPreamble; }

	static uint32 calcCombMask( const std::vector< std::string > &flags );
	
	ShaderResource( const std::string &name, int flags );
	~ShaderResource();
	
	void initDefault();
	void release();
	bool load( const char *data, int size );
	void preLoadCombination( uint32 combMask );
	void compileContexts();
	ShaderCombination *getCombination( ShaderContext &context, uint32 combMask );

	ShaderContext *findContext( const std::string &name )
	{
		for( uint32 i = 0; i < _contexts.size(); ++i )
			if( _contexts[i].id == name ) return &_contexts[i];
		
		return 0x0;
	}

	std::vector< ShaderContext > &getContexts() { return _contexts; }

	friend class Renderer;
};

typedef SmartResPtr< ShaderResource > PShaderResource;

#endif //_egShader_H_
