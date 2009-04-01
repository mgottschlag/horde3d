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

#ifndef _utPlatform_H_
#define _utPlatform_H_

#ifndef PLATFORM_WIN
#	if defined( WIN32 ) || defined( _WINDOWS )
#		define PLATFORM_WIN
#	endif
#endif

#ifndef PLATFORM_MAC
#   if defined( __APPLE__ ) || defined( __APPLE_CC__ )
#      define PLATFORM_MAC
#   endif
#endif



#ifndef DLLEXP
#	ifdef PLATFORM_WIN
#		define DLLEXP extern "C" __declspec( dllexport )
#	else
#		define DLLEXP extern "C"
#	endif
#endif

#ifndef PLATFORM_WIN
# define _stricmp strcasecmp
# define _mkdir( name ) mkdir( name, 0755 )
#endif

#ifndef _MSC_VER
# define strncpy_s( dst, dstSize, src, count ) strncpy( dst, src, count < dstSize ? count : dstSize )
#endif

// Runtime assertion
#if defined( _DEBUG )
#	define ASSERT( exp ) assert( exp )
#else
#	define ASSERT( exp )
#endif

// Static compile-time assertion
namespace StaticAssert
{
	template< bool > struct FAILED;
	template<> struct FAILED< true > { };
}
#define ASSERT_STATIC( exp ) (StaticAssert::FAILED< (exp) != 0 >())


// Check the size of some common types
// Note: this function is never called but just wraps the compile time asserts
static void __ValidatePlatform__()
{
	ASSERT_STATIC( sizeof( int ) == 4 );
	ASSERT_STATIC( sizeof( short ) == 2 );
	ASSERT_STATIC( sizeof( char ) == 1 );
}

#endif // _utPlatform_H_
