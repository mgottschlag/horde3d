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

#include "app.h"
#include "Horde3DUtils.h"
#include "Horde3DTerrain.h"
#include <math.h>
#include <iomanip>

using namespace std;

// Convert from degrees to radians
inline float degToRad( float f ) 
{
	return f * (3.1415926f / 180.0f);
}


Application::Application( const string &contentDir )
{
	for( unsigned int i = 0; i < 320; ++i ) _keys[i] = false;

	_x = 512; _y = 120; _z = 512; _rx = 0; _ry = 225; _velocity = 10.0f;
	_curFPS = 30;

	_freeze = false; _showStats = false; _debugViewMode = false; _wireframeMode = false;
	_cam = 0;

	_contentDir = contentDir;
}


bool Application::init()
{	
	// Initialize engine
	if( !Horde3D::init() )
	{	
		Horde3DUtils::dumpMessages();
		return false;
	}

	// Set options
	Horde3D::setOption( EngineOptions::LoadTextures, 1 );
	Horde3D::setOption( EngineOptions::TexCompression, 0 );
	Horde3D::setOption( EngineOptions::MaxAnisotropy, 4 );
	Horde3D::setOption( EngineOptions::ShadowMapSize, 2048 );
	Horde3D::setOption( EngineOptions::FastAnimation, 1 );

	// Add resources
	// Pipeline
	ResHandle pipeRes = Horde3D::addResource( ResourceTypes::Pipeline, "pipelines/forward.pipeline.xml", 0 );
	// Overlays
	_fontMatRes = Horde3D::addResource( ResourceTypes::Material, "overlays/font.material.xml", 0 );
	_panelMatRes = Horde3D::addResource( ResourceTypes::Material, "overlays/panel.material.xml", 0 );
	_logoMatRes = Horde3D::addResource( ResourceTypes::Material, "overlays/logo.material.xml", 0 );
	// Terrain
	ResHandle terrainRes = Horde3D::addResource( ResourceTypes::SceneGraph, "terrains/terrain1/terrain1.scene.xml", 0 );
	
	
	// Load resources
	Horde3DUtils::loadResourcesFromDisk( _contentDir.c_str() );

	// Add scene nodes
	// Add camera
	_cam = Horde3D::addCameraNode( RootNode, "Camera", pipeRes );
	// Add terrain
	NodeHandle terrain = Horde3D::addNodes( RootNode, terrainRes );
	
	/*// Add light source
	NodeHandle light = Horde3D::addLightNode( RootNode, "Light1", 0, "LIGHTING", "SHADOWMAP" );
	Horde3D::setNodeTransform( light, 512, 700, -256, -120, 0, 0, 1, 1, 1 );
	Horde3D::setNodeParamf( light, LightNodeParams::Radius, 2000 );
	Horde3D::setNodeParamf( light, LightNodeParams::FOV, 90 );
	Horde3D::setNodeParami( light, LightNodeParams::ShadowMapCount, 3 );
	Horde3D::setNodeParamf( light, LightNodeParams::ShadowSplitLambda, 0.5f );
	Horde3D::setNodeParamf( light, LightNodeParams::ShadowMapBias, 0.005f );
	Horde3D::setNodeParamf( light, LightNodeParams::Col_R, 1.0f );
	Horde3D::setNodeParamf( light, LightNodeParams::Col_G, 0.9f );
	Horde3D::setNodeParamf( light, LightNodeParams::Col_B, 0.7f );*/

	// Set sun direction for ambient pass
	NodeHandle matRes = Horde3D::findResource( ResourceTypes::Material, "terrains/terrain1/terrain1.material.xml" );
	Horde3D::setMaterialUniform( matRes, "sunDir", 1, -1, 0, 0 );

	return true;
}


void Application::mainLoop( float fps )
{
	_curFPS = fps;

	keyHandler();
	
	Horde3D::setOption( EngineOptions::DebugViewMode, _debugViewMode ? 1.0f : 0.0f );
	Horde3D::setOption( EngineOptions::WireframeMode, _wireframeMode ? 1.0f : 0.0f );
	
	if( !_freeze )
	{
	}
	
	// Set camera parameters
	Horde3D::setNodeTransform( _cam, _x, _y, _z, _rx ,_ry, 0, 1, 1, 1 );
	
	if( _showStats )
	{
		Horde3DUtils::showFrameStats( _fontMatRes, _panelMatRes, _curFPS );
	}

	// Show logo
	Horde3D::showOverlay( 0.75f, 0.8f, 0, 1, 0.75f, 1, 0, 0,
	                      1, 1, 1, 0, 1, 0.8f, 1, 1,
	                      1, 1, 1, 1, _logoMatRes, 7 );
	
	// Render scene
	Horde3D::render( _cam );

	// Finish rendering of frame
	Horde3D::finalizeFrame();

	// Remove all overlays
	Horde3D::clearOverlays();

	// Write all mesages to log file
	Horde3DUtils::dumpMessages();
}


void Application::release()
{
	// Release engine
	Horde3D::release();
}


void Application::resize( int width, int height )
{
	// Resize viewport
	Horde3D::setupViewport( 0, 0, width, height, true );
	
	// Set virtual camera parameters
	Horde3D::setupCameraView( _cam, 45.0f, (float)width / height, 0.5f, 2048.0f );
}


void Application::keyPressEvent( int key )
{
	if( key== 32 )		// Space
		_freeze = !_freeze;
	
	if( key == 264 )	// F7
		_debugViewMode = !_debugViewMode;
	
	if( key == 265 )	// F8
		_wireframeMode = !_wireframeMode;
	
	if( key == 266 )	// F9
		_showStats = !_showStats;
}


void Application::keyHandler()
{
	float curVel = _velocity / _curFPS;
	
	if( _keys[287] ) curVel *= 10;	// LShift
	
	if( _keys['W'] )
	{
		// Move forward
		_x -= sinf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
		_y -= sinf( -degToRad( _rx ) ) * curVel;
		_z -= cosf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
	}

	if( _keys['S'] )
	{
		// Move backward
		_x += sinf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
		_y += sinf( -degToRad( _rx ) ) * curVel;
		_z += cosf( degToRad( _ry ) ) * cosf( -degToRad( _rx ) ) * curVel;
	}

	if( _keys['A'] )
	{
		// Strafe left
		_x += sinf( degToRad( _ry - 90) ) * curVel;
		_z += cosf( degToRad( _ry - 90 ) ) * curVel;
	}

	if( _keys['D'] )
	{
		// Strafe right
		_x += sinf( degToRad( _ry + 90 ) ) * curVel;
		_z += cosf( degToRad( _ry + 90 ) ) * curVel;
	}
}


void Application::mouseMoveEvent( float dX, float dY )
{
	// Look left/right
	_ry -= dX / 100 * 30;
	
	// Loop up/down but only in a limited range
	_rx += dY / 100 * 30;
	if( _rx > 90 ) _rx = 90; 
	if( _rx < -90 ) _rx = -90;
}
