
#pragma once

#include "Horde3D.h"

#ifndef DLL
#	if defined( WIN32 ) || defined( _WINDOWS )
#		define DLL extern "C" __declspec( dllimport )
#	else
#		define DLL extern "C"
#	endif
#endif

const int SNT_WaterNode = 149;

struct WaterNodeParams
{
	enum List
	{
		MaterialRes = 10000,
		MeshQuality
	};
};

namespace Horde3DWater
{
	DLL NodeHandle addWaterNode( NodeHandle parent, const char *name, ResHandle materialRes );
}

