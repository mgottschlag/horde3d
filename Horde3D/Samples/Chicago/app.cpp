// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
//
// Sample Application
// --------------------------------------
// Copyright (C) 2006-2009 Nicolas Schulz
//
//
// This sample source file is not covered by the LGPL as the rest of the SDK
// and may be used without any restrictions
//
// *************************************************************************************************

#include "app.h"
#include "Horde3D.h"
#include "Horde3DUtils.h"
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

	_x = 15; _y = 3; _z = 20; _rx = -10; _ry = 60; _velocity = 10.0f;
	_curFPS = 30;

	_statMode = 0;
	_freeze = false; _debugViewMode = false; _wireframeMode = false;
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
	// Pipelines
	_forwardPipeRes = Horde3D::addResource( ResourceTypes::Pipeline, "pipelines/forward.pipeline.xml", 0 );
	_deferredPipeRes = Horde3D::addResource( ResourceTypes::Pipeline, "pipelines/deferred.pipeline.xml", 0 );
	// Overlays
	_fontMatRes = Horde3D::addResource( ResourceTypes::Material, "overlays/font.material.xml", 0 );
	_panelMatRes = Horde3D::addResource( ResourceTypes::Material, "overlays/panel.material.xml", 0 );
	_logoMatRes = Horde3D::addResource( ResourceTypes::Material, "overlays/logo.material.xml", 0 );
	// Shader for deferred shading
	ResHandle lightMatRes = Horde3D::addResource( ResourceTypes::Material, "materials/light.material.xml", 0 );
	// Environment
	ResHandle envRes = Horde3D::addResource( ResourceTypes::SceneGraph, "models/platform/platform.scene.xml", 0 );
	// Skybox
	ResHandle skyBoxRes = Horde3D::addResource( ResourceTypes::SceneGraph, "models/skybox/skybox.scene.xml", 0 );
	
	// Load resources
	Horde3DUtils::loadResourcesFromDisk( _contentDir.c_str() );

	// Add scene nodes
	// Add camera
	_cam = Horde3D::addCameraNode( RootNode, "Camera", _forwardPipeRes );
	//Horde3D::setNodeParami( _cam, CameraNodeParams::OcclusionCulling, 1 );
	// Add environment
	NodeHandle env = Horde3D::addNodes( RootNode, envRes );
	Horde3D::setNodeTransform( env, 0, 0, 0, 0, 0, 0, 0.23f, 0.23f, 0.23f );
	// Add skybox
	NodeHandle sky = Horde3D::addNodes( RootNode, skyBoxRes );
	Horde3D::setNodeTransform( sky, 0, 0, 0, 0, 0, 0, 210, 50, 210 );
	// Add light source
	NodeHandle light = Horde3D::addLightNode( RootNode, "Light1", lightMatRes, "LIGHTING", "SHADOWMAP" );
	Horde3D::setNodeTransform( light, 0, 20, 50, -30, 0, 0, 1, 1, 1 );
	Horde3D::setNodeParamf( light, LightNodeParams::Radius, 200 );
	Horde3D::setNodeParamf( light, LightNodeParams::FOV, 90 );
	Horde3D::setNodeParami( light, LightNodeParams::ShadowMapCount, 3 );
	Horde3D::setNodeParamf( light, LightNodeParams::ShadowSplitLambda, 0.9f );
	Horde3D::setNodeParamf( light, LightNodeParams::ShadowMapBias, 0.001f );
	Horde3D::setNodeParamf( light, LightNodeParams::Col_R, 0.9f );
	Horde3D::setNodeParamf( light, LightNodeParams::Col_G, 0.7f );
	Horde3D::setNodeParamf( light, LightNodeParams::Col_B, 0.75f );

	_crowdSim = new CrowdSim( _contentDir );
	_crowdSim->init();

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
		_crowdSim->update( _curFPS );
	}
	
	// Set camera parameters
	Horde3D::setNodeTransform( _cam, _x, _y, _z, _rx ,_ry, 0, 1, 1, 1 );
	
	// Show stats
	Horde3DUtils::showFrameStats( _fontMatRes, _panelMatRes, _statMode );
	if( _statMode > 0 )
	{
		if( Horde3D::getNodeParami( _cam, CameraNodeParams::PipelineRes ) == _forwardPipeRes )
			Horde3DUtils::showText( "Pipeline: forward", 0.03f, 0.24f, 0.026f, 1, 1, 1, _fontMatRes, 5 );
		else
			Horde3DUtils::showText( "Pipeline: deferred", 0.03f, 0.24f, 0.026f, 1, 1, 1, _fontMatRes, 5 );
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
	delete _crowdSim; _crowdSim = 0x0;
	
	// Release engine
	Horde3D::release();
}


void Application::resize( int width, int height )
{
	// Resize viewport
	Horde3D::setupViewport( 0, 0, width, height, true );
	
	// Set virtual camera parameters
	Horde3D::setupCameraView( _cam, 45.0f, (float)width / height, 0.1f, 1000.0f );
}


void Application::keyPressEvent( int key )
{
	if( key == 32 )		// Space
		_freeze = !_freeze;
	
	if( key == 260 )	// F3
	{
		if( Horde3D::getNodeParami( _cam, CameraNodeParams::PipelineRes ) == _forwardPipeRes )
			Horde3D::setNodeParami( _cam, CameraNodeParams::PipelineRes, _deferredPipeRes );
		else
			Horde3D::setNodeParami( _cam, CameraNodeParams::PipelineRes, _forwardPipeRes );
	}
	
	if( key == 264 )	// F7
		_debugViewMode = !_debugViewMode;

	if( key == 265 )	// F8
		_wireframeMode = !_wireframeMode;
	
	if( key == 266 )	// F9
	{
		_statMode += 1;
		if( _statMode > Horde3DUtils::MaxStatMode ) _statMode = 0;
	}
}


void Application::keyHandler()
{
	float curVel = _velocity / _curFPS;

	if( _keys[287] ) curVel *= 5;	// LShift
	
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
