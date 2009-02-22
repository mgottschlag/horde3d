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

#ifndef _egTextures_H_
#define _egTextures_H_

#include "egPrerequisites.h"
#include "egResource.h"
#include "egRendererBase.h"

struct RenderBuffer;


// =================================================================================================
// Texture Resource
// =================================================================================================

struct TextureResParams
{
	enum List
	{
		PixelData = 700,
		TexType,
		TexFormat,
		Width,
		Height
	};
};

// =================================================================================================

class TextureResource : public Resource
{
protected:
	
	RenderBuffer          *_rendBuf;	// Used when texture is renderable
	TextureTypes::List    _texType;
	TextureFormats::List  _texFormat;
	int                   _width, _height;
	uint32                _texObject;
	bool                  _hasMipMaps;

	bool raiseError( const std::string &msg );

public:
	
	static uint32 defTex2DObject;
	static uint32 defTexCubeObject;

	static void initializationFunc();
	static void releaseFunc();
	static Resource *factoryFunc( const std::string &name, int flags )
		{ return new TextureResource( name, flags ); }
	
	TextureResource( const std::string &name, int flags );
	TextureResource( const std::string &name, int flags,
	                 uint32 width, uint32 height, bool renderable );
	~TextureResource();
	
	void initDefault();
	void release();
	bool load( const char *data, int size );
	bool updateData( int param, const void *data, int size );
	float *downloadImageData();

	int getParami( int param );

	RenderBuffer *getRenderBuffer()  { return _rendBuf; }
	TextureTypes::List getTexType() { return _texType; }
	TextureFormats::List getTexFormat() { return _texFormat; }
	uint32 getWidth() const { return _width; }
	uint32 getHeight() const { return _height; } 
	uint32 getTexObject() { return _texObject; }
	bool hasMipMaps() { return _hasMipMaps; }

	friend class ResourceManager;
};

typedef SmartResPtr< TextureResource > PTextureResource;

#endif // _egTextures_H_
