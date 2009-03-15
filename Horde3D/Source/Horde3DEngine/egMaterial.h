// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2009 Nicolas Schulz
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

#ifndef _egMaterial_H_
#define _egMaterial_H_

#include "egPrerequisites.h"
#include "egResource.h"
#include "egShader.h"
#include "egTextures.h"


// =================================================================================================
// Material Resource
// =================================================================================================

struct MaterialResParams
{
	enum List
	{
		Class = 400,
		Link,
		Shader
	};	
};

// =================================================================================================

struct MatSampler
{
	std::string       name;
	PTextureResource  texRes;
};


struct MatUniform
{
	std::string  name;
	float        values[4];	


	MatUniform()
	{
		values[0] = 0; values[1] = 0; values[2] = 0; values[3] = 0;
	}
};

// =================================================================================================

class MaterialResource;
typedef SmartResPtr< MaterialResource > PMaterialResource;

class MaterialResource : public Resource
{
private:

	PShaderResource             _shaderRes;
	uint32                      _combMask;
	std::string                 _class;
	std::vector< MatSampler >   _samplers;
	std::vector< MatUniform >   _uniforms;
	std::vector< std::string >  _shaderFlags;
	PMaterialResource           _matLink;

	bool raiseError( const std::string &msg, int line = -1 );

public:

	static Resource *factoryFunc( const std::string &name, int flags )
		{ return new MaterialResource( name, flags ); }
	
	MaterialResource( const std::string &name, int flags );
	~MaterialResource();
	Resource *clone();
	
	void initDefault();
	void release();
	bool load( const char *data, int size );
	bool setUniform( const std::string &name, float a, float b, float c, float d );
	bool setSampler( const std::string &name, TextureResource *texRes );
	bool isOfClass( const std::string &theClass );

	int getParami( int param );
	bool setParami( int param, int value );
	const char *getParamstr( int param );
	bool setParamstr( int param, const char *value );

	friend class ResourceManager;
	friend class Renderer;
	friend class MeshNode;
};

#endif // _egMaterial_H_
