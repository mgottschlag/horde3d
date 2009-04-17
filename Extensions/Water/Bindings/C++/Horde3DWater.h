
#pragma once

#include "Horde3D.h"

#ifndef DLL
#	if defined( WIN32 ) || defined( _WINDOWS )
#		define DLL extern "C" __declspec( dllimport )
#	else
#		define DLL extern "C"
#	endif
#endif

// Noise resource
const int RT_NoiseResource = 148;

struct NoiseResParams
{
	enum List
	{
		Octaves = 10000,
		NormalMap
	};
};

// Water scene node
const int SNT_WaterNode = 149;

struct WaterNodeParams
{
	enum List
	{
		MaterialRes = 10000,
		NoiseRes,
		GridWidth,
		GridHeight,
		UseGPU
	};
};

namespace Horde3DWater
{
	DLL ResHandle addNoise( const char *name, int octaves );
	DLL void setNoiseTime( ResHandle noise, float time );
	DLL float getNoiseHeight( ResHandle noise, float x, float z);

	DLL int getGPUWaterSupported( void );

	DLL NodeHandle addWaterNode( NodeHandle parent, const char *name, ResHandle noiseRes, ResHandle materialRes );
	DLL NodeHandle addWaterNodeGPU( NodeHandle parent, const char *name, ResHandle noiseRes, ResHandle materialRes );
}

