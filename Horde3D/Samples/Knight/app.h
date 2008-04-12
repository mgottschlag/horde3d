// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
//
// Sample Application
// --------------------------------------
// Copyright (C) 2006-2008 Nicolas Schulz
//
//
// This sample source file is not covered by the LGPL as the rest of the SDK
// and may be used without any restrictions
//
// *************************************************************************************************

#ifndef _app_H_
#define _app_H_

#include "Horde3D.h"
#include <sstream>
using namespace std;


class Application
{
private:

	bool			_keys[320];
	float			_x, _y, _z, _rx, _ry;	// Viewer position and orientation
	float			_velocity;				// Velocity for movement
	float			_curFPS, _timer;
	stringstream	_fpsText;

	bool			_freeze, _showFPS, _debugViewMode, _wireframeMode;
	float			_animTime, _weight;
	
	// Engine objects
	ResHandle		_pipeRes, _fontMatRes, _logoMatRes, _hdrPipeRes, _forwardPipeRes;
	NodeHandle		_cam, _knight, _particleSys;

	void keyHandler();

public:
	
	Application();
	
	bool init();
	void mainLoop( float fps );
	void release();
	void resize( int width, int height );

	void keyPressEvent( int key );
	void keyStateChange( int key, bool state ) { if( key >= 0 && key < 320 ) _keys[key] = state; }
	void mouseMoveEvent( float dX, float dY );
};

#endif // _app_H_
