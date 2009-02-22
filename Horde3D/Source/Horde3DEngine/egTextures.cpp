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

#include "egTextures.h"
#include "egModules.h"
#include "utImage.h"

#include "utDebug.h"

using namespace std;


// *************************************************************************************************
// Class TextureResource
// *************************************************************************************************

#define FOURCC( c0, c1, c2, c3 ) ((c0) | (c1<<8) | (c2<<16) | (c3<<24))

#define DDSD_MIPMAPCOUNT      0x00020000

#define DDPF_ALPHAPIXELS      0x00000001
#define DDPF_FOURCC           0x00000004
#define DDPF_RGB              0x00000040

#define DDSCAPS2_CUBEMAP      0x00000200
#define DDSCAPS2_CM_COMPLETE  0x00000400 | 0x00000800 | 0x00001000 | 0x00002000 | 0x00004000 | 0x00008000

#define D3DFMT_A16B16G16R16F  113
#define D3DFMT_A32B32G32R32F  116


struct DDSHeader
{
	uint32  dwMagic;
	uint32  dwSize;
	uint32  dwFlags;
	uint32  dwHeight, dwWidth;
	uint32  dwPitchOrLinearSize;
	uint32  dwDepth;
	uint32  dwMipMapCount;
	uint32  dwReserved1[11];

	struct {
		uint32  dwSize;
		uint32  dwFlags;
		uint32  dwFourCC;
		uint32  dwRGBBitCount;
		uint32  dwRBitMask, dwGBitMask, dwBBitMask, dwABitMask;
	} pixFormat;

	struct {
		uint32  dwCaps, dwCaps2, dwCaps3, dwCaps4;
	} caps;

	uint32  dwReserved2;
} ddsHeader;


uint32 TextureResource::defTex2DObject = 0;
uint32 TextureResource::defTexCubeObject = 0;


void TextureResource::initializationFunc()
{
	unsigned char defaultTexture[] = 
		{	255,192,128, 255,192,128, 255,192,128, 255,192,128,
			255,192,128, 255,192,128, 255,192,128, 255,192,128,
			255,192,128, 255,192,128, 255,192,128, 255,192,128,
			255,192,128, 255,192,128, 255,192,128, 255,192,128	};

	// Upload default textures
	defTex2DObject = Modules::renderer().uploadTexture( TextureTypes::Tex2D, defaultTexture, 4, 4,
		TextureFormats::RGB8, 0, 0, true, false );

	defTexCubeObject = Modules::renderer().uploadTexture( TextureTypes::TexCube, defaultTexture, 4, 4,
		TextureFormats::RGB8, 0, 0, true, false );
	for( uint32 i = 1; i < 6; ++i ) 
	{
		Modules::renderer().uploadTexture( TextureTypes::TexCube, defaultTexture, 4, 4,
			TextureFormats::RGB8, i, 0, true, false, defTexCubeObject );
	}
}


void TextureResource::releaseFunc()
{
	Modules::renderer().unloadTexture( defTex2DObject, TextureTypes::Tex2D );
	Modules::renderer().unloadTexture( defTexCubeObject, TextureTypes::TexCube );
}


TextureResource::TextureResource( const string &name, int flags ) :
	Resource( ResourceTypes::Texture, name, flags )
{
	_texType = TextureTypes::Tex2D;
	initDefault();
}


TextureResource::TextureResource( const string &name, int flags,
                                  uint32 width, uint32 height, bool renderable ) :
	Resource( ResourceTypes::Texture, name, flags ), _rendBuf( 0x0 ), _texType( TextureTypes::Tex2D ),
	_texFormat( TextureFormats::RGBA8 ), _width( width ), _height( height )
{	
	_loaded = true;
	
	if( !renderable )
	{
		_hasMipMaps = !(_flags & ResourceFlags::NoTexMipmaps);
		_texObject = Modules::renderer().uploadTexture( _texType, 0x0, _width, _height, _texFormat, 0, 0,
			_hasMipMaps, !(_flags & ResourceFlags::NoTexCompression) );
		
		if( _texObject == 0 ) initDefault();
	}
	else
	{
		_rendBuf = new RenderBuffer();
		*_rendBuf = Modules::renderer().createRenderBuffer( width, height, RenderBufferFormats::RGBA8, false, 1, 0 ); 
		_texObject = _rendBuf->colBufs[0];
	}
}


TextureResource::~TextureResource()
{
	release();
}


void TextureResource::initDefault()
{
	_rendBuf = 0x0;
	_texFormat = TextureFormats::RGBA8;
	_width = 0; _height = 0;
	_hasMipMaps = true;
	
	if( _texType = TextureTypes::TexCube )
		_texObject = defTexCubeObject;
	else
		_texObject = defTex2DObject;
}


void TextureResource::release()
{
	if( _rendBuf != 0x0 )
	{
		// In this case _texObject is just points to the render buffer
		Modules::renderer().destroyRenderBuffer( *_rendBuf );
		delete _rendBuf; _rendBuf = 0x0;
	}
	else if( _texObject != 0 && _texObject != defTex2DObject && _texObject != defTexCubeObject )
	{
		Modules::renderer().unloadTexture( _texObject, _texType );
	}

	_texObject = 0;
}


bool TextureResource::raiseError( const string &msg )
{
	// Reset
	release();
	initDefault();

	Modules::log().writeError( "Texture resource '%s': %s", _name.c_str(), msg.c_str() );
	
	return false;
}


bool TextureResource::load( const char *data, int size )
{
	if( !Resource::load( data, size ) ) return false;
	if( !Modules::config().loadTextures ) return true;

	// Check if image is a dds
	if( size > 128 && *((uint32 *)data) == FOURCC( 'D', 'D', 'S', ' ' ) )
	{
		// Load dds
		ASSERT_STATIC( sizeof( DDSHeader ) == 128 );

		memcpy( &ddsHeader, data, 128 );
		
		// Check header
		// There are some flags that are required to be set for every dds but we don't check them
		if( ddsHeader.dwSize != 124 )
		{
			return raiseError( "Invalid DDS header" );
		}

		// Store properties
		_width = ddsHeader.dwWidth;
		_height = ddsHeader.dwWidth;
		_texFormat = (TextureFormats::List)-1;
		_texObject = 0;
		int mipCount = ddsHeader.dwFlags & DDSD_MIPMAPCOUNT ? ddsHeader.dwMipMapCount : 1;
		_hasMipMaps = mipCount > 1 ? true : false;

		// Get texture type
		if( ddsHeader.caps.dwCaps2 == 0 )
		{
			_texType = TextureTypes::Tex2D;
		}
		else if( ddsHeader.caps.dwCaps2 & DDSCAPS2_CUBEMAP )
		{
			if( !(ddsHeader.caps.dwCaps2 & DDSCAPS2_CM_COMPLETE) )
				raiseError( "DDS cubemap does not contain all cube sides" );
			_texType = TextureTypes::TexCube;
		}
		else
		{
			return raiseError( "Unsupported DDS texture type" );
		}
		
		// Get pixel format
		if( ddsHeader.pixFormat.dwFlags & DDPF_FOURCC )
		{
			switch( ddsHeader.pixFormat.dwFourCC )
			{
			case FOURCC( 'D', 'X', 'T', '1' ):
				_texFormat = TextureFormats::DXT1;
				break;
			case FOURCC( 'D', 'X', 'T', '3' ):
				_texFormat = TextureFormats::DXT3;
				break;
			case FOURCC( 'D', 'X', 'T', '5' ):
				_texFormat = TextureFormats::DXT5;
				break;
			case D3DFMT_A16B16G16R16F: 
				_texFormat = TextureFormats::RGBA16F;
				break;
			case D3DFMT_A32B32G32R32F: 
				_texFormat = TextureFormats::RGBA32F;
				break;
			}
		}
		else if( ddsHeader.pixFormat.dwFlags & DDPF_RGB )
		{
			const int L_BGR = 0;
			const int L_RGB = 1;
			int layout = -1;

			if( ddsHeader.pixFormat.dwRBitMask == 0x00ff0000 &&
			    ddsHeader.pixFormat.dwGBitMask == 0x0000ff00 &&
			    ddsHeader.pixFormat.dwBBitMask == 0x000000ff ) layout = L_BGR;
			else
			if( ddsHeader.pixFormat.dwRBitMask == 0x00ff0000 &&
			    ddsHeader.pixFormat.dwGBitMask == 0x0000ff00 &&
			    ddsHeader.pixFormat.dwBBitMask == 0x000000ff ) layout = L_RGB;

			if( layout == L_BGR || layout == L_RGB )
			{
				if( ddsHeader.pixFormat.dwRGBBitCount == 24 )
				{
					_texFormat = layout == L_BGR ? TextureFormats::BGR8 : TextureFormats::RGB8;
				}
				else if( ddsHeader.pixFormat.dwRGBBitCount == 32 )
				{
					if( !(ddsHeader.pixFormat.dwFlags & DDPF_ALPHAPIXELS) ||
					    ddsHeader.pixFormat.dwABitMask == 0x00000000 )
					{
						_texFormat = layout == L_BGR ? TextureFormats::BGRX8 : TextureFormats::RGBX8;
					}
					else
					{	
						_texFormat = layout == L_BGR ? TextureFormats::BGRA8 : TextureFormats::RGBA8;
					}
				}
			}
		}

		if( (int)_texFormat < 0 ) return raiseError( "Unsupported DDS pixel format" );

		// Upload texture subresources
		int numSlices = _texType == TextureTypes::TexCube ? 6 : 1;
		char *pixels = (char *)(data + 128);

		for( int i = 0; i < numSlices; ++i )
		{
			int width = _width, height = _height;

			for( int j = 0; j < mipCount; ++j )
			{
				size_t mipSize =  Modules::renderer().calcTexSize( _texFormat, width, height );
				if( pixels + mipSize > (char*)data + size )
					return raiseError( "Corrupt DDS" );
				
				_texObject = Modules::renderer().uploadTexture( _texType, pixels, width, height,
					_texFormat, i, j, false, false, _texObject );

				pixels += mipSize;
				width >>= 1;
				height >>= 1;
			}
		}

		ASSERT( pixels == (char *)data + (size - 1) );
	}
	else
	{
		// Load image using stbi
		bool hdr = false;
		if( stbi_is_hdr_from_memory( (unsigned char *)data, size ) > 0 ) hdr = true;
		
		int comps;
		void *pixels = 0x0;
		if( hdr )
			pixels = stbi_loadf_from_memory( (unsigned char *)data, size, &_width, &_height, &comps, 4 );
		else
			pixels = stbi_load_from_memory( (unsigned char *)data, size, &_width, &_height, &comps, 4 );

		if( pixels == 0x0 )
			return raiseError( "Invalid image format (" + string( stbi_failure_reason() ) + ")" );
		
		_texType = TextureTypes::Tex2D;
		_texFormat = hdr ? TextureFormats::RGBA16F : TextureFormats::RGBA8;
		_hasMipMaps = !(_flags & ResourceFlags::NoTexMipmaps);
		
		// Upload texture
		_texObject = Modules::renderer().uploadTexture( _texType, pixels, _width, _height, _texFormat, 0, 0,
			_hasMipMaps, !(_flags & ResourceFlags::NoTexCompression) );

		stbi_image_free( pixels );
	}

	if( _texObject == 0 ) return raiseError( "Failed to upload texture map" );

	return true;
}


bool TextureResource::updateData( int param, const void *data, int size )
{
	if( Resource::updateData( param, data, size ) ) return true;
	
	if( param == TextureResParams::PixelData )
	{
		if( _texType != TextureTypes::Tex2D ) return false;
		if( _texObject == 0 ) return false;
		if( size != _width * _height * 4 ) return false;

		Modules::renderer().updateTexture2D( (unsigned char *)data, _width, _height, 4, _texObject );

		return true;
	}

	return false;
}


float *TextureResource::downloadImageData()
{
	if( _texType != TextureTypes::Tex2D ) return 0x0;

	int width, height;
	return Modules::renderer().downloadTexture2DData( _texObject, &width, &height );
}


int TextureResource::getParami( int param )
{
	switch( param )
	{
	case TextureResParams::TexType:
		return _texType;
	case TextureResParams::TexFormat:
		return _texFormat;
	case TextureResParams::Width:
		return _width;
	case TextureResParams::Height:
		return _height;
	default:
		return Resource::getParami( param );
	}
}
