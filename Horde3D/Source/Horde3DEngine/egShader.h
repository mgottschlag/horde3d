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

struct XMLNode;


// =================================================================================================
// Code Resource
// =================================================================================================

class CodeResource : public Resource
{
private:
	
	std::string  _code;

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

	bool isLoaded() { return _loaded; }
	const std::string &getCode() { return _code; }

	friend class Renderer;
};

typedef SmartResPtr< CodeResource > PCodeResource;


// =================================================================================================
// Shader Resource
// =================================================================================================

struct ShaderCodeFract
{
	PCodeResource  refCodeRes;
	std::string    code;
};


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


struct ShaderContext
{
	std::string                     id;
	uint32                          shaderObject;
	uint32                          lastUpdateStamp;
	
	// RenderConfig
	BlendModes::List                blendMode;
	bool                            writeDepth;
	
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
	int                             attrib_normal, attrib_tangent, attrib_bitangent;
	int                             attrib_joints, attrib_weights;
	int                             attrib_texCoords0, attrib_texCoords1;

	// Custom uniforms
	std::map< std::string, int >    customUniforms;

	std::vector< ShaderCodeFract >  vertShaderFracts, fragShaderFracts;
	bool                            compiled;


	ShaderContext()
	{
		compiled = false;
		shaderObject = 0;
		lastUpdateStamp = 0;
		writeDepth = true;
		blendMode = BlendModes::Replace;
	}
};

// =================================================================================================

class ShaderResource : public Resource
{
private:
	
	static std::string            _vertPreamble, _fragPreamble;
	
	std::vector< ShaderContext >  _contexts;

	bool raiseError( const std::string &msg, int line = -1 );
	bool parseCode( XMLNode &node, std::vector< ShaderCodeFract > &codeFracts );

public:
	
	static Resource *factoryFunc( const std::string &name, int flags )
		{ return new ShaderResource( name, flags ); }

	static void setPreambles( const std::string &vertPreamble, const std::string &fragPreamble )
		{ _vertPreamble = vertPreamble; _fragPreamble = fragPreamble; }
	
	ShaderResource( const std::string &name, int flags );
	~ShaderResource();
	
	void initDefault();
	void release();
	bool load( const char *data, int size );
	void compileShaders();

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
