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

#ifndef _utTimer_H_
#define _utTimer_H_

#include "utPlatform.h"

#ifdef PLATFORM_WIN
#   define WIN32_LEAN_AND_MEAN 1
#	define NOMINMAX
#   include <windows.h>
#else
#	include <sys/time.h>
#endif


class Timer
{
protected:

	double         _startTime;
	double         _elapsedTime;

#ifdef PLATFORM_WIN
	LARGE_INTEGER  _timerFreq;
	DWORD          _affMask;
#endif

	bool           _enabled;
	

	double getTime()
	{
	#ifdef PLATFORM_WIN
		// Make sure that time is read from the same CPU
		DWORD_PTR threadAffMask = SetThreadAffinityMask( GetCurrentThread(), _affMask );

		// Avoid the reordering of instructions by emitting a serialization instruction
		#ifdef _MSC_VER
		   _asm { CPUID };
		#else
		   asm volatile("cpuid");
		#endif
		
		// Read high performance counter
		LARGE_INTEGER curTick;
		QueryPerformanceCounter( &curTick );

		// Restore affinity mask
		SetThreadAffinityMask( GetCurrentThread(), threadAffMask );

		return (double)curTick.QuadPart / (double)_timerFreq.QuadPart * 1000.0;
	#else
		timeval tv;
		gettimeofday( &tv, 0x0 );
		return (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0;
	#endif
	}

public:

	Timer() : _elapsedTime( 0 ), _enabled( false )
	{
	#ifdef PLATFORM_WIN
		// Get timer frequency
		QueryPerformanceFrequency( &_timerFreq );
		
		// Find first available CPU
		DWORD procMask, sysMask;
		GetProcessAffinityMask( GetCurrentProcess(), &procMask, &sysMask );
		_affMask = 1;
		while( (_affMask & procMask) == 0 ) _affMask <<= 1;
	#endif
	}
	
	void setEnabled( bool enabled )
	{	
		if( enabled && !_enabled )
		{
			_startTime = getTime();
			_enabled = true;
		}
		else if( !enabled && _enabled )
		{
			double endTime = getTime();
			_elapsedTime += endTime - _startTime;
			_enabled = false;
		}
	}

	void reset()
	{
		_elapsedTime = 0;
		if( _enabled ) _startTime = getTime();
	}
	
	float getElapsedTimeMS()
	{
		if( _enabled )
		{
			double endTime = getTime();
			_elapsedTime += endTime - _startTime;
			_startTime = endTime;
		}

		return (float)_elapsedTime;
	}
};

#endif  // _utTimer_H_
