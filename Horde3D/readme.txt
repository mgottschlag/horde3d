Horde3D 
-------	
	Next-Generation Graphics Engine
	
		-------
		-S D K-
		-------
		
		Version 1.0.0 Beta3
		

Copyright (C) 2006-2009 Nicolas Schulz and the Horde3D Team

http://www.horde3d.org

Contact: See website

	
The complete SDK is licensed under the terms of the GNU Lesser General Public License (LGPL).
See the documentation for more information.

Horde3D requires an OpenGL 2.0 compatible graphics card with the latest drivers.
A GeForce 6 or Radeon X1000 series card is the minimum requirement to run the samples.

The source code of the engine and tools is included in the SDK. It has the following dependencies:

	- XMLParser by Frank Vanden Berghen
		http://www.applied-mathematics.net/tools/xmlParser.html
	- stbi by Sean Barrett
		http://nothings.org
	- GLFW 2.6 for window management in samples
		http://glfw.sourceforge.net/
		
These libraries are included directly as code in the SDK.


Release Notes:
	
	The new Beta3 release brings a few substantial improvements. It features a refactored
	shader system with a cleaner and more powerful FX-like file format and support for automatic
	shader permutation generation. The performance of Beta3 was optimized a lot, for example by
	introducing an enhanced LOD system. Another new feature is support for loading textures
	from DDS files. Besides these major upgrades, there are many samller API improvements,
	optimizations and bug fixes. Finally, Beta3 got improvements to the Collada Converter
	so that it now supports a wider range of DCC tools.


Special thanks go to the University of Augsburg for supporting this project!




------------------------------------------------------------------------------------------
Description of included samples
------------------------------------------------------------------------------------------

Chicago
-------

This sample uses a simple crowd simulation to show how Horde3D can
be used to render environments and a plenty of animated models.
The demo supports forward and deferred rendering which can be
selected by modifying the pipeline_Chicago.xml file.

Input:

	Use WASD to move and the mouse to look around.
	Hold down LSHIFT to move faster.
	Space freezes the scene.
	F1 sets fullscreen mode.
	F3 switches between forward and deferred shading.
	F7 toggles debug view.
	F8 toggles wireframe mode.
	F9 toggles frame stats display.
	ESC quits the application.

Notes on content:

	The character model was created by Sirda from TurboSquid.
	Some of the textures are taken from the ATI Radeon SDK and are thus
	copyrighted by ATI Technologies, Inc.
	

	
Knight
------

This sample shows a knight model with reflections coming from a cube map. The
pipeline applies high dynamic range lighting with simple tone mapping and a
bloom effect. The knight has a particle system attached to the tip of his sword
which results in some interesting graphical effects. Using the keyboard it is
possible to blend seamlessly between two animations.


Input:

	Use WASD to move and the mouse to look around.
	Hold down LSHIFT to move faster.
	Use 1 and 2 to blend between character animations.
	Space freezes the scene.
	F1 sets fullscreen mode.
	F3 switches between hdr and standard forward lighting.
	F7 toggles debug view.
	F8 toggles wireframe mode.
	F9 toggles frame stats and information display.
	ESC quits the application.

Notes on content:

	The character model and animations were created by Rob Galanakis (www.robg3d.com)
	and are distributed under the terms of the Creative Commons Attribution-
	Noncommercial 3.0 License (http://creativecommons.org/licenses/by-nc/3.0/).
	The cubemap texture is a modified version of one of M@dcow's high res skymaps
	which can be found at BlenderArtist.org.
	
	
	
-------------------------
Have fun with the engine!
-------------------------
