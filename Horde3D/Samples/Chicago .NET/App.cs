// *************************************************************************************************
//
// Chicago .NET - sample application for Horde3D .NET wrapper
// ----------------------------------------------------------
//
// Copyright (C) 2006-07 Nicolas Schulz and Martin Burkhard
//
// This file is intended for use as a code example, and may be used, modified, 
// or distributed in source or object code form, without restriction. 
// This sample is not covered by the LGPL.
//
// The code and information is provided "as-is" without warranty of any kind, 
// either expressed or implied.
//
// *************************************************************************************************

using System;
using System.Windows.Forms;
using Horde3DNET;

namespace Horde3DNET.Samples.ChicagoNET
{
    internal class Application
    {
	    private float			_x, _y, _z, _rx, _ry;	// Viewer position and orientation
	    private float			_velocity;				// Velocity for movement
	    private float			_curFPS, _timer;
        private string          _fpsText;

        private bool            _freeze, _showStats, _debugViewMode, _wireframeMode;

	    private CrowdSim		_crowdSim;
    	
	    // Engine objects
        private int             _fontMatRes, _panelMatRes, _logoMatRes;

        private int             _cam, _deferredPipeRes, _forwardPipeRes;
                
        // workaround
        private bool _initialized = false;

    	
        // Convert from degrees to radians
        public static float degToRad( float f ) 
        {
	        return f * (3.1415926f / 180.0f);
        }

	    public Application()
        {
	        _x = 0; _y = 2; _z = 0; _rx = 0; _ry = 0; _velocity = 10.0f;
            _curFPS = 0; _timer = 0.0f;
            _freeze = false; _showStats = false; _debugViewMode = false;
            _fpsText = string.Empty;
        }

        public bool init()
        {
	        // Initialize engine
            if (!Horde3D.init())
            {
                Horde3DUtils.dumpMessages();
                return false;
            }

	        // Set options
	        Horde3D.setOption( Horde3D.EngineOptions.LoadTextures, 1 );
	        Horde3D.setOption( Horde3D.EngineOptions.TexCompression, 0 );
	        Horde3D.setOption( Horde3D.EngineOptions.MaxAnisotropy, 4 );
	        Horde3D.setOption( Horde3D.EngineOptions.ShadowMapSize, 2048 );
            Horde3D.setOption( Horde3D.EngineOptions.FastAnimation, 1 );

            // Add resources
 	        // Pipelines
            _forwardPipeRes = Horde3D.addResource((int)Horde3D.ResourceTypes.Pipeline, "pipelines/forward.pipeline.xml", 0);
            _deferredPipeRes = Horde3D.addResource((int)Horde3D.ResourceTypes.Pipeline, "pipelines/deferred.pipeline.xml", 0);
            // Overlays
            _fontMatRes = Horde3D.addResource((int)Horde3D.ResourceTypes.Material, "overlays/font.material.xml", 0);            
            _panelMatRes = Horde3D.addResource( (int) Horde3D.ResourceTypes.Material, "overlays/panel.material.xml", 0 );
            _logoMatRes = Horde3D.addResource((int)Horde3D.ResourceTypes.Material, "overlays/logo.material.xml", 0);
            // Shader for deferred shading
            int lightMatRes = Horde3D.addResource((int) Horde3D.ResourceTypes.Material, "materials/light.material.xml", 0);
            // Environment
            int envRes = Horde3D.addResource((int) Horde3D.ResourceTypes.SceneGraph, "models/platform/platform.scene.xml", 0);
            // Skybox
            int skyBoxRes = Horde3D.addResource((int) Horde3D.ResourceTypes.SceneGraph, "models/skybox/skybox.scene.xml", 0);


            // Load resources
            Horde3DUtils.loadResourcesFromDisk( "../Content" );


            // Add scene nodes
	        // Add camera
	        _cam = Horde3D.addCameraNode( Horde3D.RootNode, "Camera", _forwardPipeRes );
            // Add environment
            int env = Horde3D.addNodes( Horde3D.RootNode, envRes);
            Horde3D.setNodeTransform( env, 0, 0, 0, 0, 0, 0, 0.23f, 0.23f, 0.23f );
	        // Add skybox
            int sky = Horde3D.addNodes(Horde3D.RootNode, skyBoxRes);
	        Horde3D.setNodeTransform( sky, 0, 0, 0, 0, 0, 0, 210, 50, 210 );
            // Add light source

            int light = Horde3D.addLightNode(Horde3D.RootNode, "Light1", lightMatRes, "LIGHTING", "SHADOWMAP");
            Horde3D.setNodeTransform( light, 0, 20, 50, -30, 0, 0, 1, 1, 1 );
            Horde3D.setNodeParamf(light, (int) Horde3D.LightNodeParams.Radius, 200);
            Horde3D.setNodeParamf(light, (int) Horde3D.LightNodeParams.FOV, 90);
            Horde3D.setNodeParami(light, (int) Horde3D.LightNodeParams.ShadowMapCount, 3);
            Horde3D.setNodeParamf(light, (int) Horde3D.LightNodeParams.ShadowSplitLambda, 0.9f);
            Horde3D.setNodeParamf(light, (int) Horde3D.LightNodeParams.ShadowMapBias, 0.001f);
            Horde3D.setNodeParamf(light, (int) Horde3D.LightNodeParams.Col_R, 0.9f);
            Horde3D.setNodeParamf(light, (int) Horde3D.LightNodeParams.Col_G, 0.7f);
            Horde3D.setNodeParamf(light, (int) Horde3D.LightNodeParams.Col_B, 0.75f);

            _crowdSim = new CrowdSim();        
	        _crowdSim.init();

            _initialized = true;

	        return true;
        }

        public void mainLoop(float fps)
        {
	        _curFPS = fps;
            _timer += 1 / fps;
            
	        Horde3D.setOption( Horde3D.EngineOptions.WireframeMode, _wireframeMode ? 1.0f : 0.0f );
            Horde3D.setOption( Horde3D.EngineOptions.DebugViewMode, _debugViewMode ? 1.0f : 0.0f );
        	
	        if( !_freeze )
	        {
		        _crowdSim.update( _curFPS );
	        }

            // Set camera parameters
            Horde3D.setNodeTransform(_cam, _x, _y, _z, _rx, _ry, 0, 1, 1, 1);

            if (_showStats)
            {
                Horde3DUtils.showFrameStats(_fontMatRes, _panelMatRes, _curFPS);
            }
            
            // Show logo
            Horde3D.showOverlay(0.75f, 0.8f, 0, 1, 0.75f, 1, 0, 0,
                                1, 1, 1, 0, 1, 0.8f, 1, 1,
                                1, 1, 1, 1, _logoMatRes, 7);


            // Render scene
            Horde3D.render(_cam);

            // Finish rendering of frame
            Horde3D.finalizeFrame();

            // Clear Overlays
            Horde3D.clearOverlays();

            // Write all messages to log file
            Horde3DUtils.dumpMessages();
        }

        public void release()
        {      	
	        // Release engine
	        Horde3D.release();
        }

        public void resize(int width, int height)
        {
            if (!_initialized) return;

            // Resize viewport
            Horde3D.setupViewport( 0, 0, width, height, true );

            // Set virtual camera parameters
            Horde3D.setupCameraView(_cam, 45.0f, (float) width/height, 0.1f, 1000.0f);
        }

        public void keyPressEvent(Keys key)
        {
            switch (key)
            {
                case Keys.Space:
                    _freeze = !_freeze;
                    break;

                case Keys.F3:
                    if(  Horde3D.getNodeParami( _cam, (int) Horde3D.CameraNodeParams.PipelineRes ) == _forwardPipeRes )
                        Horde3D.setNodeParami(_cam, (int) Horde3D.CameraNodeParams.PipelineRes, _deferredPipeRes);
                    else
                        Horde3D.setNodeParami(_cam, (int) Horde3D.CameraNodeParams.PipelineRes, _forwardPipeRes);
                    break;

                case Keys.F7:
                    _debugViewMode = !_debugViewMode;
                    break;

                case Keys.F8:
                    _wireframeMode = !_wireframeMode;
                    break;

                case Keys.F9:
                    _showStats = !_showStats;
                    break;
            }
        }

        public void keyHandler()
        {
            float curVel = _velocity / _curFPS;

            if(InputManager.IsKeyDown(Keys.W))
            {
                // Move forward
                _x -= (float)Math.Sin(degToRad(_ry)) * (float)Math.Cos(-degToRad(_rx)) * curVel;
                _y -= (float)Math.Sin(-degToRad(_rx)) * curVel;
                _z -= (float)Math.Cos(degToRad(_ry)) * (float)Math.Cos(-degToRad(_rx)) * curVel;
            }

            if (InputManager.IsKeyDown(Keys.S))
            {
                // Move backward
                _x += (float)Math.Sin(degToRad(_ry)) * (float)Math.Cos(-degToRad(_rx)) * curVel;
                _y += (float)Math.Sin(-degToRad(_rx)) * curVel;
                _z += (float)Math.Cos(degToRad(_ry)) * (float)Math.Cos(-degToRad(_rx)) * curVel;
            }

            if(InputManager.IsKeyDown(Keys.A))
            {                
                // Strafe left
                _x += (float)Math.Sin(degToRad(_ry - 90)) * curVel;
                _z += (float)Math.Cos(degToRad(_ry - 90)) * curVel;
            }

            if(InputManager.IsKeyDown(Keys.D))
            { 
                // Strafe right
                _x += (float)Math.Sin(degToRad(_ry + 90)) * curVel;
                _z += (float)Math.Cos(degToRad(_ry + 90)) * curVel;
            }
        }

        public void mouseMoveEvent(float dX, float dY)
        {
            // Look left/right
            _ry -= dX / 100 * 30;

            // Loop up/down but only in a limited range
            _rx -= dY / 100 * 30;
            if (_rx > 90) _rx = 90;
            if (_rx < -90) _rx = -90;
        }

    }
}
