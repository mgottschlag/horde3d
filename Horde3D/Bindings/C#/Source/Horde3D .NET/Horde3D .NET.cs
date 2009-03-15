// *************************************************************************************************
//
// Horde3D .NET wrapper
// ----------------------------------
// Copyright (C) 2007 Martin Burkhard
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

using System;
using System.IO;
using System.Runtime.Serialization;
using System.Security.Permissions;
using System.Runtime.InteropServices;
using Horde3DNET.Properties;

[assembly: SecurityPermission(SecurityAction.RequestMinimum, Unrestricted = true)]
[assembly: CLSCompliant(true)]
namespace Horde3DNET
{
    public static class Horde3D
    {

        // Predefined constants
        private static int _rootNode = 1;

        public static int RootNode
        {
            get { return _rootNode; }
        }

        public enum EngineOptions
        {
            MaxLogLevel = 1,
            MaxNumMessages,
            TrilinearFiltering,
            MaxAnisotropy,
            TexCompression,
            LoadTextures,
            FastAnimation,
            ShadowMapSize,
            SampleCount,
            WireframeMode,
            DebugViewMode,
            DumpFailedShaders
        }

        public enum EngineStats
        {
            TriCount = 100,
            BatchCount,
            LightPassCount,
            FrameTime,
            CustomTime
        }

        public enum ResourceTypes
        {
            Undefined = 0,
            SceneGraph,
            Geometry,
            Animation,
            Material,
            Code,
            Shader,
            Texture,
            ParticleEffect,
            Pipeline
        }

        // Flags
        public enum ResourceFlags
        {
            NoQuery = 1, //horde3d 1.0
            NoTexCompression = 2, //horde3d 1.0
            NoTexMipmaps = 4 //horde3d 1.0
        }

        public enum GeometryResParams
        {
       		VertexCount = 200,
		    IndexCount,
		    VertexData,
		    IndexData
        }

        public enum AnimationResParams
        {
            FrameCount = 300
        }

        public enum MaterialResParams
        {
            Class = 400,
            Link,
            Shader
        }

        public enum TextureResParams
        {
            PixelData = 700,
            TexType,
            TexFormat,
            Width,
            Height
        }

        public enum EffectResParams
        {
            LifeMin = 900,
            LifeMax,
            MoveVelMin,
            MoveVelMax,
            MoveVelEndRate,
            RotVelMin,
            RotVelMax,
            RotVelEndRate,
            SizeMin,
            SizeMax,
            SizeEndRate,
            Col_R_Min,
            Col_R_Max,
            Col_R_EndRate,
            Col_G_Min,
            Col_G_Max,
            Col_G_EndRate,
            Col_B_Min,
            Col_B_Max,
            Col_B_EndRate,
            Col_A_Min,
            Col_A_Max,
            Col_A_EndRate
        }

        public enum SceneNodeTypes
        {
            Undefined = 0,
            Group,
            Model,
            Mesh,
            Joint,
            Light,
            Camera,
            Emitter
        }

        public enum SceneNodeParams
        {
            Name = 1,
            AttachmentString
        }

        public enum GroupNodeParams
        {
            MinDist = 100, //horde3d 1.0
            MaxDist
        }

        public enum ModelNodeParams
        {
            GeometryRes = 200,
            SoftwareSkinning,
            LodDist1,
            LodDist2,
            LodDist3,
            LodDist4
        }

        public enum MeshNodeParams
        {
            MaterialRes = 300,
            BatchStart,
            BatchCount,
            VertRStart,
            VertREnd,
            LodLevel
        }

        public enum JointNodeParams
        {
            JointIndex = 400
        }

        public enum LightNodeParams
        {
            MaterialRes = 500,
            Radius,
            FOV,
            Col_R,
            Col_G,
            Col_B,
            ShadowMapCount,
            ShadowSplitLambda,
            ShadowMapBias
        }

        public enum CameraNodeParams
        {
            PipelineRes = 600,
            OutputTex,
            OutputBufferIndex,
            LeftPlane,
            RightPlane,
            BottomPlane,
            TopPlane,
            NearPlane,
            FarPlane,
            Orthographic,
            OcclusionCulling
        }

        public enum EmitterNodeParams
        {
            MaterialRes = 700,
            ParticleEffectRes,
            MaxCount,
            RespawnCount,
            Delay,
            EmissionRate,
            SpreadAngle,
            ForceX,
            ForceY,
            ForceZ
        }

        // --- Basic funtions ---
        /// <summary>
        /// This function returns a string containing the current version of Horde3D.
        /// </summary>
        /// <returns>The version string</returns>
        public static string getVersionString()
        {
            IntPtr ptr = NativeMethodsEngine.getVersionString();
            //Console.WriteLine(Marshal.PtrToStringAnsi(ptr));
            return Marshal.PtrToStringAnsi(ptr);
        }

        /// <summary>
        /// This function checks if a specified extension is contained in the DLL/shared object of the engine.
        /// </summary>
        /// <param name="extensionName">name of the extension</param>
        /// <returns>true if extension is implemented, otherwise false</returns>
        public static bool checkExtension(string extensionName)
        {
            return NativeMethodsEngine.checkExtension(extensionName);
        }

        /// <summary>
        /// This function initializes the graphics engine and makes it ready for use. It has to be the
		/// first call to the engine except for getVersionString. In order to successfully initialize
		/// the engine the calling application must provide a valid OpenGL context. The function can be
		/// called several times on different rendering contexts in order to initialize them.
        /// </summary>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool init()
        {
            if (getVersionString() != Resources.VersionString)
                throw new LibraryIncompatibleException(Resources.LibraryIncompatibleExceptionString);

            return NativeMethodsEngine.init();
        }

        /// <summary>
        /// This function releases the engine and frees all objects and associated memory. 
        /// It should be called when the application is destroyed.
        /// </summary>
        public static void release()
        {
            NativeMethodsEngine.release();
        }

        /// <summary>
        /// This function sets the location and size of the viewport. It has to be called
        /// after engine initialization and whenever the size of the rendering context/window
        /// changes. The resizeBuffers parameter specifies whether render targets with a size
        /// relative to the viewport dimensions should be resized. This is usually desired
        /// after engine initialization and when the window is resized but not for just rendering
        /// to a part of the framebuffer.
        /// </summary>
        /// <param name="x">the x-position of the viewport in the rendering context</param>
        /// <param name="y">the y-position of the viewport in the rendering context</param>
        /// <param name="width">the width of the viewport</param>
        /// <param name="height">the height of the viewport</param>
        /// <param name="resizeBuffers">specifies whether render targets should be adapted to new size</param>
        public static void setupViewport(int x, int y, int width, int height, bool resizeBuffers)
        {
            NativeMethodsEngine.setupViewport(x, y, width, height,resizeBuffers);
        }

        /// <summary>
        /// This is the main function of the engine. 
        /// It executes all the rendering, animation and other tasks. 
        /// The function can be called several times per frame, 
        /// for example in order to write to different
        /// output buffers.
        /// <param name="node">camera node used for rendering scene</param>
        /// <returns>true in case of success, otherwise false</returns>
        /// </summary>       
        public static bool render(int node)
        {
            return NativeMethodsEngine.render(node);
        }

        /// <summary>
        /// This function tells the engine that the current frame is finished and that all
        /// subsequent rendering operations will be for the next frame.
        /// </summary>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool finalizeFrame()
        {
            return NativeMethodsEngine.finalizeFrame();
        }

        /// <summary>
        /// This function removes all nodes from the scene graph except the root node and releases all resources.
        /// Warning: All resource and node IDs are invalid after calling this function.         
        /// </summary>
        public static void clear()
        {
            NativeMethodsEngine.clear();
        }


        // --- General functions ---

        /// <summary>
        /// This function returns the next message string from the message queue and writes additional information to the specified variables. If no message is left over in the queue an empty string is returned.
        /// </summary>
        /// <param name="level">pointer to variable for storing message level indicating importance (can be NULL)</param>
        /// <param name="time">pointer to variable for stroing time when message was added (can be NULL)</param>
        /// <returns>message string or empty string if no message is in queue</returns>
        public static string getMessage(out int level, out float time)
        {
            IntPtr ptr = NativeMethodsEngine.getMessage(out level, out time);
            return Marshal.PtrToStringAnsi(ptr);
        }

        /// <summary>
        /// This function gets a specified option parameter and returns its value.
        /// </summary>
        /// <param name="param">option parameter</param>
        /// <returns>current value of the specified option parameter</returns>
        public static float getOption(EngineOptions param)
        {
            return NativeMethodsEngine.getOption(param);
        }

        /// <summary>
        /// This function sets a specified option parameter to a specified value.
        /// </summary>
        /// <param name="param">option parameter</param>
        /// <param name="value">value of the option parameter</param>
        /// <returns>true if the option could be set to the specified value, otherwise false</returns>
        public static bool setOption(EngineOptions param, float value)
        {
            return NativeMethodsEngine.setOption( (int) param, value);
        }

        /// <summary>
        /// Gets a statistic value of the engine.
        /// </summary>
        /// This function returns the value of the specified statistic. The reset flag makes it possible
        /// to reset the statistic value after reading.
        /// <param name="param">statistic parameter</param>
        /// <param name="reset">flag specifying whether statistic value should be reset</param>
        /// <returns>current value of the specified statistic parameter</returns>
        public static float getStat(EngineStats param, bool reset)
        {
            return NativeMethodsEngine.getStat((int)param, reset);
        }

        /// <summary>
        /// This function displays an overlay with a specified material at a specified position on the screen.
        /// </summary>
        /// <remarks>
        /// An overlay is a 2D image that can be used to render 2D GUI elements. 
        /// The coordinate system used has its origin (0, 0) at the lower left corner of the screen and its maximum (1, 1) at the upper right corner. 
        /// Texture coordinates are using the same system, where the coordinates (0, 0) correspond to the lower left corner of the image. 
        /// Overlays can have different layers which describe the order in which they are drawn. 
        /// Overlays with smaller layer numbers are drawn before overlays with higher layer numbers.
        /// </remarks>
        /// <param name="x_ll">x position of the lower left corner</param>
        /// <param name="y_ll">y position of the lower left corner</param>
        /// <param name="u_ll">u texture coordinate of the lower left corner</param>
        /// <param name="v_ll">v texture coordinate of the lower left corner</param>
        /// <param name="x_lr">x position of the lower right corner</param>
        /// <param name="y_lr">y position of the lower right corner</param>
        /// <param name="u_lr">u texture coordinate of the lower right corner</param>
        /// <param name="v_lr">v texture coordinate of the lower right corner</param>
        /// <param name="x_ur">x position of the upper right corner</param>
        /// <param name="y_ur">y position of the upper right corner</param>
        /// <param name="u_ur">u texture coordinate of the upper right corner</param>
        /// <param name="v_ur">v texture coordinate of the upper right corner</param>
        /// <param name="x_ul">x position of the upper left corner</param>
        /// <param name="y_ul">y position of the upper left corner</param>
        /// <param name="u_ul">u texture coordinate of the upper left corner</param>
        /// <param name="v_ul">v texture coordinate of the upper left corner</param>
        /// <param name="colR">red color value of the overlay that is set for the material's shader</param>
        /// <param name="colG">green color value of the overlay that is set for the material's shader</param>
        /// <param name="colB">blue color value of the overlay that is set for the material's shader</param>
        /// <param name="colA">alpha color value of the overlay that is set for the material's shader</param>
        /// <param name="material">material resource used for rendering</param>
        /// <param name="layer">layer index of the overlay (Values: from 0 to 7)</param>
        public static void showOverlay(float x_ll, float y_ll, float u_ll, float v_ll,
                                        float x_lr, float y_lr, float u_lr, float v_lr,
                                        float x_ur, float y_ur, float u_ur, float v_ur,
                                        float x_ul, float y_ul, float u_ul, float v_ul,
                                        float colR, float colG, float colB, float colA,
                                        int layer, int material)
        {
            NativeMethodsEngine.showOverlay(x_ll, y_ll, u_ll, v_ll, x_lr, y_lr, u_lr, v_lr, x_ur, y_ur, u_ur, v_ur, x_ul, y_ul, u_ul, v_ul, colR, colG, colB, colA, material, layer);
        }

        /// <summary>
        /// Clears the overlays added before using showOverlay
        /// </summary>
        public static void clearOverlays()
        {
            NativeMethodsEngine.clearOverlays();
        }


        // --- Resource functions ---
        /// <summary>
        /// This function returns the type of a specified resource. 
        /// If the resource handle is invalid, the function returns the resource type 'Unknown'.
        /// </summary>
        /// <param name="res">handle to the resource whose type will be returned</param>
        /// <returns>type of the scene node</returns>
        public static int getResourceType(int res)
        {
            return NativeMethodsEngine.getResourceType(res);
        }

        /// <summary>
        /// Returns the name of a resource.
        /// </summary>
        /// This function returns a pointer to the name of a specified resource. If the resource handle
        /// is invalid, the function returns an empty string.        
        /// <param name="res">handle to the resource</param>
        /// <returns>name of the resource or empty string in case of failure</returns>
        public static string getResourceName(int res)
        {
            return Marshal.PtrToStringAnsi(NativeMethodsEngine.getResourceName(res));
        }

        /// <summary>
        /// This function searches the resource of the specified type and name and returns its handle. If
        /// the resource is not available in the resource manager a zero handle is returned.
        /// </summary>
        /// <param name="type">type of the resource</param>
        /// <param name="start">name of the resource</param>
        /// <returns></returns>
        public static int getNextResource(int type, int start)
        {
            return (int)NativeMethodsEngine.getNextResource(type, start);
        }

        /// <summary>
        /// This function searches the resource of the specified type and name and returns its handle. If the resource is not available in the resource manager a zero handle is returned.
        /// </summary>
        /// <remarks>
        /// The content path of the specified ResourceType is added automatically.
        /// </remarks>
        /// <param name="type">type of the resource</param>
        /// <param name="name">name of the resource</param>
        /// <returns>handle to the resource or 0 if not found</returns>
        public static int findResource(int type, string name)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);
            return (int)NativeMethodsEngine.findResource(type, name);
        }

        /// <summary>
        /// This function tries to add a resource of a specified type and name to the resource manager. 
        /// If a resource of the same type and name is already found, the handle to the existing resource is returned instead of creating a new one.
        /// </summary>
        /// <param name="type">type of the resource</param>
        /// <param name="name">name of the resource</param>
        /// <param name="flags">flags used for creating the resource</param>
        /// <returns>handle to the resource to be added or 0 in case of failure</returns>
        public static int addResource(int type, string name, int flags)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);

            return (int)NativeMethodsEngine.addResource(type, name, flags);
        }

        /// <summary>
        /// Duplicates a resource.
        /// </summary>
        /// This function duplicates a specified resource. In the cloning process a new resource with the
        /// specified name is added to the resource manager and filled with the data of the specified source
        /// resource. If the specified name for the new resource is already in use, the function fails and
        /// returns 0. If the name string is empty, a unique name for the resource is generated automatically.
        /// <remarks>*Note: The name string may not contain a colon character (:)*</remarks>
        /// <param name="sourceRes">handle to resource to be cloned</param>
        /// <param name="name">name of new resource (can be empty for auto-naming)</param>
        /// <returns>handle to the cloned resource or 0 in case of failure</returns>
        public static int cloneResource(int sourceRes, string name)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);
            return (int)NativeMethodsEngine.cloneResource(sourceRes, name);
        }

        /// <summary>
        /// Removes a resource.
        /// </summary>
        /// This function decreases the user reference count of a specified resource. When the user reference
        /// count is zero and there are no internal references, the resource can be released and removed using
        /// the API fuction releaseUnusedResources.
        /// <param name="res">handle to the resource to be removed</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static int removeResource(int res)
        {
            return NativeMethodsEngine.removeResource(res);
        }

        /// <summary>
        /// Checks if a resource is loaded.
        /// </summary>
        /// This function checks if the specified resource has been successfully loaded.
        /// <param name="res">handle to the resource to be checked</param>
        /// <returns>true if resource is loaded, otherwise or in case of failure false</returns>
        public static bool isResourceLoaded(int res)
        {
            return NativeMethodsEngine.isResourceLoaded(res);
        }

        /// <summary>
        /// Loads a resource.
        /// </summary>
        /// <remarks>
        /// This function loads data for a resource that was previously added to the resource manager.
		/// If data is a NULL-pointer the resource manager is told that the resource doesn't have any data
		/// (e.g. the corresponding file was not found). In this case, the resource remains in the unloaded state
		/// but is no more returned when querying unloaded resources. When the specified resource is already loaded,
        /// the function returns false.
        /// 
        /// *Important Note: XML-data must be NULL-terminated*
        /// </remarks>
        /// <param name="name">res handle to the resource for which data will be loaded</param>
        /// <param name="data">the data to be loaded</param>
        /// <param name="size">size of the data block</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool loadResource(int res, byte[] data, int size)
        {            
            if (data == null) throw new ArgumentNullException("data");

            if (data.Length < size)
                throw new ArgumentException(Resources.LoadResourceArgumentExceptionString, "data");

            // allocate memory for resource data
            IntPtr ptr = Marshal.AllocHGlobal(size + 1);

            // copy byte data into allocated memory
            Marshal.Copy(data, 0, ptr, size);

            // terminate data block
            Marshal.WriteByte(ptr, size, 0x00);

            // load resource
            bool result = NativeMethodsEngine.loadResource(res, ptr, size);

            // free previously allocated memory
            Marshal.FreeHGlobal(ptr);

            return result;
        }

        /// <summary>
        /// This function unloads a previously loaded resource and restores the default values it had before loading. The state is set back to unloaded which makes it possible to load the resource again.
        /// </summary>
        /// <param name="res">handle to resource to be unloaded</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool unloadResource(int res)
        {
            return NativeMethodsEngine.unloadResource((int) res);
        }

        /// <summary>
        /// Gets a property of a resource.
        /// </summary>
        /// This function returns a specified property of the specified resource.
		/// The property must be of the type int.
        /// <param name="res">handle to the resource to be accessed</param>
        /// <param name="param">parameter to be accessed</param>
        /// <returns>value of the parameter</returns>
        public static int getResourceParami(int res, int param)
        {
            return NativeMethodsEngine.getResourceParami(res, param);
        }

        /// <summary>
        /// Sets a property of a resource.
        /// </summary>
        /// This function sets a specified property of the specified resource to a specified value.
        /// The property must be of the type int.
        /// <param name="res">handle to the node to be modified</param>
        /// <param name="param">parameter to be modified</param>
        /// <param name="value">new value for the specified parameter</param>
        /// <returns>true in case of success otherwise false</returns>
        public static bool setResourceParami(int res, int param, int value)
        {
            return NativeMethodsEngine.setResourceParami(res, param, value);
        }

        /// <summary>
        /// Gets a property of a resource.
        /// </summary>
        /// This function returns a specified property of the specified resource.
        /// The property must be of the type float.
        /// <param name="res">handle to the resource to be accessed</param>
        /// <param name="param">parameter to be accessed</param>
        /// <returns>value of the parameter</returns>
        public static float getResourceParamf(int res, int param)
        {
            return NativeMethodsEngine.getResourceParamf(res, param);
        }

        /// <summary>
        /// Sets a property of a resource.
        /// </summary>
        /// This function sets a specified property of the specified resource to a specified value.
        /// The property must be of the type float.
        /// <param name="res">handle to the node to be modified</param>
        /// <param name="param">parameter to be modified</param>
        /// <param name="value">new value for the specified parameter</param>
        /// <returns>true in case of success otherwise false</returns>
        public static bool setResourceParamf(int res, int param, float value)
        {
            return NativeMethodsEngine.setResourceParamf(res, param, value);
        }

        /// <summary>
        /// Gets a property of a resource.
        /// </summary>
        /// This function returns a specified property of the specified resource.
        /// The property must be of the type string (const char *).
        ///         
        /// <param name="res">handle to the resource to be accessed</param>
        /// <param name="param">parameter to be accessed</param>
        /// <returns>value of the property or empty string if no such property exists</returns>
        public static string getResourceParamstr(int res, int param)
        {
            return Marshal.PtrToStringAnsi(NativeMethodsEngine.getResourceParamstr(res, param));
        }

        /// <summary>
        /// Sets a property of a resource.
        /// </summary>
        /// This function sets a specified property of the specified resource to a specified value.
        /// The property must be of the type string (const char *).
        /// <param name="res">handle to the node to be modified</param>
        /// <param name="param">parameter to be modified</param>
        /// <param name="value">new value for the specified parameter</param>
        /// <returns>true in case of success otherwise false</returns>
        public static bool setResourceParamstr(int res, int param, string value)
        {
            return NativeMethodsEngine.setResourceParamstr(res, param, value);
        }

        /// <summary>
        /// This function returns a pointer to the specified data of a specified resource. 
        /// For information on the format (int, float, ..) of the pointer see the ResourceData description. 
        /// </summary>
        /// <param name="res">handle to the resource to be accessed</param>
        /// <param name="param">parameter indicating data of the resource that will be accessed</param>
        /// <returns>specified resource data if it is available</returns>
        public static IntPtr getResourceData(int res, int param)
        {
            return NativeMethodsEngine.getResourceData(res, param);
        }
        
        /// <summary>
        /// This function updates the content of a resource that was successfully loaded before.
        /// The new data must have exactly the same data layout as the data that was loaded.
        /// </summary>
        /// <remarks>Notes on available ResourceData parameters: 
        /// Tex2DPixelData - Sets the image data of a Texture2D resource. 
        /// The data must point to a memory block that contains the pixels of the image. 
        /// Each pixel needs to have 32 bit color data in BGRA format. 
        /// The dimensions of the image (width, height) must be exactly the same as the dimensions 
        /// of the image that was originally loaded for the resource.
        /// PLEASE NOTE: calls to updateResourceData are not threadsafe and so it might not work in case you have a separated render thread.</remarks>
        /// <param name="res">handle to the resource for which the data is modified</param>
        /// <param name="param">data structure which will be updated</param>
        /// <param name="data">the new data</param>
        /// <param name="size">size of the new data block</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool updateResourceData(int res, int param, byte[] data, int size)
        {
            if (data == null)
                throw new ArgumentNullException("data");

            if (data.Length < size)
                throw new ArgumentException(Resources.LoadResourceArgumentExceptionString, "data");

            // allocate memory for resource data
            IntPtr ptr = Marshal.AllocHGlobal(size + 1);

            // copy byte data into allocated memory
            Marshal.Copy(data, 0, ptr, size);

            // terminate string
            Marshal.WriteByte(ptr, size, 0x00);

            // load resource
            bool result = NativeMethodsEngine.updateResourceData(res, param, ptr, size);

            // free previously allocated memory
            Marshal.FreeHGlobal(ptr);

            return result;
        }

        /// <summary>
        /// Returns handle to an unloaded resource.
        /// </summary>
        /// This function looks for a resource that is not yet loaded and returns its handle.
        /// If there are no unloaded resources or the zero based index specified is greater than the number
        /// of the currently unloaded resources, 0 is returned.
        /// <param name="index">index of unloaded resource within the internal list of unloaded resources (starting with 0) </param>
        /// <returns>handle to an unloaded resource or 0</returns>
        public static int queryUnloadedResource(int index)
        {
            return NativeMethodsEngine.queryUnloadedResource(index);            
        }

        /// <summary>
        /// This function releases resources that are no longer used. 
        /// Unused resources were either told to be released by the user calling removeResource() or are no more referenced by any other engine objects.
        /// </summary>
        public static void releaseUnusedResources()
        {
            NativeMethodsEngine.releaseUnusedResources();
        }

        /// <summary>
        /// Adds a Texture2D resource.
        /// </summary>
        /// This function tries to create and add a Texture2D resource with the specified name to the resource
        /// manager. If a Texture2D resource with the same name is already existing, the function fails. The
        /// texture is initialized with the specified dimensions and the resource is declared as loaded. This
        /// function is especially useful to create dynamic textures (e.g. for displaying videos) or output buffers
        /// for render-to-texture.
        /// <remarks>*Note: The name string may not contain a colon character (:)*</remarks>
        /// <param name="name">name of the resource</param>
        /// <param name="flags">flags used for creating the resource</param>
        /// <param name="width">width of the texture image</param>
        /// <param name="height">height of the texture image</param>
        /// <param name="renderable">flag indicating whether the texture can be used as an output buffer for a Camera node</param>
        /// <returns>handle to the created resource or 0 in case of failure</returns>
        public static int createTexture2D(string name, int flags, int width, int height, bool renderable)
        {
            return NativeMethodsEngine.createTexture2D(name, flags, width, height, renderable);
        }

        /// <summary>
        /// Sets preambles of all Shader resources.
        /// </summary>
        /// This function defines a header that is inserted at the beginning of all shaders. The preamble
        /// is used when a shader is compiled, so changing it will not affect any shaders that are already
        /// compiled. The preamble is useful for setting platform-specific defines that can be employed for
        /// creating several shader code paths, e.g. for supporting different hardware capabilities.
        /// <param name="vertPreamble">preamble text of vertex shaders (default: empty string)</param>
        /// <param name="fragPreamble">preamble text of fragment shaders (default: empty string)</param>
        public static void setShaderPreambles(string vertPreamble, string fragPreamble)
        {
            NativeMethodsEngine.setShaderPreambles(vertPreamble, fragPreamble);
        }

        // Material specific
        /// <summary>
        /// This function sets the specified shader uniform of the specified material to the specified values.
        /// </summary>
        /// <param name="matRes">handle to the Material resource to be accessed</param>
        /// <param name="name">name of the uniform as defined in Material resource</param>
        /// <param name="a">value of first component</param>
        /// <param name="b">value of second component</param>
        /// <param name="c">value of third component</param>
        /// <param name="d">value of fourth component</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool setMaterialUniform(int matRes, string name, float a, float b, float c, float d)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);

            return NativeMethodsEngine.setMaterialUniform(matRes, name, a, b, c, d);
        }

        /// <summary>
        /// Sets the activation state of a pipeline stage.
        /// </summary>
        /// This function enables or disables a specified stage of the specified pipeline resource.
        /// <param name="pipelineRes">handle to the Pipeline resource to be accessed</param>
        /// <param name="stageName">name of the stage to be modified</param>
        /// <param name="enabled">flag indicating whether the stage shall be enabled or disabled</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool setPipelineStageActivation(int pipelineRes, string stageName, bool enabled)
        {
            return NativeMethodsEngine.setPipelineStageActivation(pipelineRes, stageName, enabled);
        }

        /// <summary>
        /// Reads the pixel data of a pipeline render target buffer.
        /// </summary>
        /// This function reads the pixels of the specified buffer of the specified render target from the specified
        /// pipeline resource and stores it in the specified float array. To calculate the size required for the array this
        /// function can be called with a NULL pointer for dataBuffer and pointers to variables where the width,
        /// height and number of (color) components (e.g. 4 for RGBA or 1 for depth) will be stored.
        /// The function is not intended to be used for real-time scene rendering but rather as a tool for debugging.
        /// For more information about the render buffers please refer to the Pipeline documentation.
        /// <param name="pipelineRes">handle to pipeline resource</param>
        /// <param name="targetName">unique name of render target to access</param>
        /// <param name="bufIndex">index of buffer to be accessed</param>
        /// <param name="width">pointer to variable where the width of the buffer will be stored (can be NULL)</param>
        /// <param name="height">pointer to variable where the height of the buffer will be stored (can be NULL</param>
        /// <param name="compCount">pointer to variable where the number of components will be stored (can be NULL)</param>
        /// <param name="dataBuffer">pointer to float array where the pixel data will be stored (can be NULL)</param>
        /// <param name="bufferSize">size of dataBuffer array in bytes</param>
        /// <returns></returns>
        //public static bool getPipelineRenderTargetData(int pipelineRes, string targetName, int bufIndex,
        //    out int width, out int height, out int compCount, float[] dataBuffer, int bufferSize)
        //{
            
        //}

        // SceneGraph functions
        /// <summary>
        /// This function returns the type of a specified scene node. 
        /// If the node handle is invalid, the function returns the node type 'Unknown'.
        /// </summary>
        /// <param name="node">handle to the scene node whose type will be returned</param>
        /// <returns>type of the scene node</returns>
        public static SceneNodeTypes getNodeType(int node)
        {
            return NativeMethodsEngine.getNodeType(node);
        }

        /// <summary>
        /// Returns the parent of a scene node.
        /// </summary>
        /// This function returns the handle to the parent node of a specified scene node. If the specified
        /// node handle is invalid or the root node, 0 is returned.
        /// <param name="node">handle to the scene node</param>
        /// <returns>handle to parent node or 0 in case of failure</returns>
        public static int getNodeParent(int node)
        {
            return NativeMethodsEngine.getNodeParent(node);
        }

        /// <summary>
        /// Relocates a node in the scene graph.
        /// </summary>
        /// This function relocates a scene node. It detaches the node from its current parent and attaches
        /// it to the specified new parent node. If the attachment to the new parent is not possible, the
        /// function returns false. Relocation is not possible for the RootNode.
        /// <param name="node">handle to the scene node to be relocated</param>
        /// <param name="parent">handle to the new parent node</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool setNodeParent(int node, int parent)
        {
            return NativeMethodsEngine.setNodeParent(node, parent);
        }

        /// <summary>
        /// Returns the handle to a child node.
        /// </summary>
        /// 
        /// This function looks for the n-th (index) child node of a specified node and returns its handle. If the child
        /// doesn't exist, the function returns 0.
        /// <param name="node">handle to the parent node</param>
        /// <param name="index">index of the child node</param>
        /// <returns>handle to the child node or 0 if child doesn't exist</returns>
        public static int getNodeChild(int node, int index)
        {
            return NativeMethodsEngine.getNodeChild(node, index);
        }

        /// <summary>
        /// This function creates several new nodes as described in a SceneGraph resource and attaches them to a specified parent node.
        /// </summary>
        /// <param name="parent">handle to parent node to which the root of the new nodes will be attached</param>
        /// <param name="res">handle to the SceneGraph resource</param>
        /// <returns>handle to the root of the created nodes or 0 in case of failure</returns>
        public static int addNodes(int parent, int res)
        {
            return (int)NativeMethodsEngine.addNodes(parent, res);
        }

        /// <summary>
        /// This function removes the specified node and all of it's children from the scene.
        /// </summary>
        /// <param name="node">handle to the node to be removed</param>
        /// <returns>true in case of success otherwise false</returns>
        public static bool removeNode(int node)
        {
            return NativeMethodsEngine.removeNode(node);
        }

        /// <summary>
        /// This function sets the activation state of the specified node to active or inactive. Inactive nodes are excluded from rendering.
        /// </summary>
        /// <param name="node">handle to the node to be modified</param>
        /// <param name="active">boolean value indicating whether node is active or inactive</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool setNodeActivation(int node, bool active)
        {            
            return NativeMethodsEngine.setNodeActivation(node, active);
        }

        /// <summary>
        /// Checks if a scene node has been transformed by the engine.
        /// </summary>
        /// This function checks if a scene node has been transformed by the engine since the last
        /// time the transformation flag was reset. Therefore, it stores a flag that is set to true when a
        /// setTransformation function is called explicitely by the application or when the node transformation
        /// has been updated by the animation system. The function also makes it possible to reset the
        /// transformation flag.
        /// <param name="node">handle to the node to be accessed</param>
        /// <param name="reset">flag indicating whether transformation flag shall be reset</param>
        /// <returns>true if node has been transformed, otherwise false</returns>
        public static bool checkNodeTransformFlag(int node, bool reset)
        {
            return NativeMethodsEngine.checkNodeTransformFlag(node, reset);
        }

        /// <summary>
        /// This function gets the translation, rotation and scale of a specified scene node object.
        /// The coordinates are in local space and contain the transformation of the node relative to its parent.
        /// </summary>
        /// <param name="node">handle to the node which will be accessed</param>
        /// <param name="px">x variable where position of the node will be stored</param>
        /// <param name="py">y variable where position of the node will be stored</param>
        /// <param name="pz">z variable where position of the node will be stored</param>
        /// <param name="rx">x variable where rotation of the node in Euler angles (degrees) will be stored</param>
        /// <param name="ry">y variable where rotation of the node in Euler angles (degrees) will be stored</param>
        /// <param name="rz">z variable where rotation of the node in Euler angles (degrees) will be stored</param>
        /// <param name="sx">x variable where scale of the node will be stored</param>
        /// <param name="sy">y variable where scale of the node will be stored</param>
        /// <param name="sz">z variable where scale of the node will be stored</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool getNodeTransform(int node, out float px, out float py, out float pz,
                                out float rx, out float ry, out float rz, out float sx, out float sy, out float sz)
        {
            return NativeMethodsEngine.getNodeTransform(node, out px, out py, out pz, out rx, out ry, out rz, out sx, out sy, out sz);
        }

        /// <summary>
        /// This function sets the relative translation, rotation and scale of a specified scene node object.
        /// The coordinates are in local space and contain the transformation of the node relative to its parent.
        /// </summary>
        /// <param name="node">handle to the node which will be modified</param>
        /// <param name="px">x position of the node</param>
        /// <param name="py">y position of the node</param>
        /// <param name="pz">z position of the node</param>
        /// <param name="rx">x rotation of the node in Euler angles (degrees)</param>
        /// <param name="ry">y rotation of the node in Euler angles (degrees)</param>
        /// <param name="rz">z rotation of the node in Euler angles (degrees)</param>
        /// <param name="sx">x scale of the node</param>
        /// <param name="sy">y scale of the node</param>
        /// <param name="sz">z scale of the node</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool setNodeTransform(int node, float px, float py, float pz,
                                float rx, float ry, float rz, float sx, float sy, float sz)
        {
            return NativeMethodsEngine.setNodeTransform(node, px, py, pz, rx, ry, rz, sx, sy, sz);
        }

        /// <summary>
        /// This function returns a pointer to the relative and absolute transformation matrices of the specified node in the specified pointer variables.
        /// </summary>
        /// <param name="node">handle to the scene node whose matrices will be accessed</param>
        /// <param name="relMat">pointer to a variable where the address of the relative transformation matrix will be stored</param>
        /// <param name="absMat">pointer to a variable where the address of the absolute transformation matrix will be stored</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool getNodeTransformMatrices(int node, out IntPtr relMat, out IntPtr absMat)
        {
            return NativeMethodsEngine.getNodeTransformMatrices(node, out relMat, out absMat);
        }

        /// <summary>
        /// This function sets the relative transformation matrix of the specified scene node. 
        /// It is basically the same as setNodeTransform but takes directly a matrix instead of individual transformation parameters.
        /// </summary>
        /// <param name="node">handle to the scene node whose matrix will be updated</param>
        /// <param name="mat4x4">array of a 4x4 matrix in column major order</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool setNodeTransformMatrix(int node, float[] mat4x4)
        {
            if (mat4x4.Length != 16) throw new ArgumentOutOfRangeException("mat4x4", Resources.MatrixOutOfRangeExceptionString);

            return NativeMethodsEngine.setNodeTransformMatrix(node, mat4x4);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="node"></param>
        /// <param name="param"></param>
        /// <returns></returns>
        public static float getNodeParamf(int node, int param)
        {
            return NativeMethodsEngine.getNodeParamf(node, param);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="node"></param>
        /// <param name="param"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        public static bool setNodeParamf(int node, int param, float value)
        {
            return NativeMethodsEngine.setNodeParamf(node, param, value);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="node"></param>
        /// <param name="param"></param>
        /// <returns></returns>
        public static int getNodeParami(int node, int param)
        {
            return NativeMethodsEngine.getNodeParami(node, param);
        }

        /// <summary>
        /// Sets a property of a scene node.
        /// </summary>
        /// <remarks>This function sets a specified property of the specified node to a specified value.
		/// The property must be of the type int or ResHandle.</remarks>
        /// <param name="node">handle to the node to be modified</param>
        /// <param name="param">parameter to be modified</param>
        /// <param name="value">new value for the specified parameter</param>
        /// <returns>true in case of success otherwise false</returns>
        public static bool setNodeParami(int node, int param, int value)
        {
            return NativeMethodsEngine.setNodeParami(node, param, value);
        }


        /// <summary>
        /// Gets a property of a scene node.
        /// </summary>
        /// This function returns a specified property of the specified node.
        /// The property must be of the type string (const char *).
        /// <param name="node">handle to the node to be accessed</param>
        /// <param name="param">parameter to be accessed</param>
        /// <returns>value of the property or empty string if no such property exists</returns>
        public static string getNodeParamstr(int node, int param)
        {
            return  Marshal.PtrToStringAnsi(NativeMethodsEngine.getNodeParamstr(node, param));
        }

        /// <summary>
        /// Sets a property of a scene node.
        /// </summary>
        /// This function sets a specified property of the specified node to a specified value.
        /// The property must be of the type string.
        /// 
        /// <param name="node">handle to the node to be modified</param>
        /// <param name="param">parameter to be modified</param>
        /// <param name="value">new value for the specified parameter</param>
        /// <returns></returns>
        public static bool setNodeParamstr(int node, int param, string value)
        {
            if( value == null) throw new ArgumentNullException("value", Resources.StringNullExceptionString);
            return NativeMethodsEngine.setNodeParamstr(node, param, value);
        }


        /// <summary>
        /// This function stores the world coordinates of the axis aligned bounding box of a specified node in the specified variables. 
        /// The bounding box is represented using the minimum and maximum coordinates on all three axes. 
        /// </summary>
        /// <param name="node">handle to the node which will be accessed</param>
        /// <param name="minX">variable where minimum x-coordinates will be stored</param>
        /// <param name="minY">variable where minimum y-coordinates will be stored</param>
        /// <param name="minZ">variable where minimum z-coordinates will be stored</param>
        /// <param name="maxX">variable where maximum x-coordinates will be stored</param>
        /// <param name="maxY">variable where maximum y-coordinates will be stored</param>
        /// <param name="maxZ">variable where maximum z-coordinates will be stored</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool getNodeAABB(int node, out float minX, out float minY, out float minZ, out float maxX, out float maxY, out float maxZ)
        {
            return NativeMethodsEngine.getNodeAABB(node, out minX, out minY, out minZ, out maxX, out maxY, out maxZ);
        }

        // added (Horde3D 1.0)
        /// <summary>
        /// Finds scene nodes with the specified properties.
        /// </summary>
        /// <remarks> This function loops recursively over all children of startNode and 
        /// adds them to an internal list of results if they match the specified name and type. 
        /// The result list is cleared each time this function is called. The function returns the 
        /// number of nodes which were found and added to the list.</remarks>
        /// <param name="node"></param>
        /// <param name="name"></param>
        /// <param name="type"></param>
        /// <returns></returns>
        public static int findNodes(int node, string name, int type)
        {
            return NativeMethodsEngine.findNodes(node, name, type);
        }

        /// <summary>
        /// Gets a result from the findNodes query.
        /// </summary>
        /// <remarks>This function returns the n-th (index) result of a previous findNodes query. The result is the handle
        /// to a scene node with the poperties specified at the findNodes query. If the index doesn't exist in the
        /// result list the function returns 0.</remarks>
        /// <param name="index">index of search result</param>
        /// <returns>handle to scene node from findNodes query or 0 if result doesn't exist</returns>
        public static int getNodeFindResult(int index)
        {
            return NativeMethodsEngine.getNodeFindResult(index);
        }
        
        /// <summary>
        /// This function checks recursively if the specified ray intersects the specified node or one of its children.
        /// The function finds intersections relative to the ray origin and returns the number of intersecting scene
        /// nodes. The ray is a line segment and is specified by a starting point (the origin) and a finite direction
        /// vector which also defines its length. Currently this function is limited to returning intersections with Meshes.        
        /// </summary>
        /// <param name="node">node at which intersection check is beginning</param>
        /// <param name="ox">ray origin</param>
        /// <param name="oy">ray origin</param>
        /// <param name="oz">ray origin</param>
        /// <param name="dx">ray direction vector also specifying ray length</param>
        /// <param name="dy">ray direction vector also specifying ray length</param>
        /// <param name="dz">ray direction vector also specifying ray length</param>
        /// <param name="numNearest">maximum number of intersection points to be stored (0 for all)</param>
        /// <returns>handle to nearest intersected node or 0 if no node was hit</returns>
        public static int castRay(int node, float ox, float oy, float oz, float dx, float dy, float dz, int numNearest)
        {
            return NativeMethodsEngine.castRay(node, ox, oy, oz, dx, dy, dz, numNearest);
        }


        /// <summary>
        /// Returns a result of a previous castRay query.
        /// </summary>
        /// This functions is used to access the results of a previous castRay query. The index is used to access
        /// a specific result. The intersection data is copied to the specified variables.
        /// <param name="index">index of result to be accessed (range: 0 to number of results returned by castRay)</param>
        /// <param name="node">handle of intersected node</param>
        /// <param name="distance">distance from ray origin to intersection point</param>
        /// <param name="intersection">coordinates of intersection point (float[3] array)</param>
        /// <returns></returns>
        public static bool getCastRayResult(int index, int node, out float distance, float[] intersection)
        {
            return NativeMethodsEngine.getCastRayResult(index, node, out distance, intersection);
        }

        /// <summary>
        /// Checks if a node is visible.
        /// </summary>
        /// <remarks>This function checks if a specified node is visible from the perspective of a specified
        /// camera. The function always checks if the node is in the camera's frustum. If checkOcclusion
        /// is true, the function will take into account the occlusion culling information from the previous
        /// frame (if occlusion culling is disabled the flag is ignored). The flag calcLod determines whether the
        /// detail level for the node should be returned in case it is visible. The function returns -1 if the node
        /// is not visible, otherwise 0 (base LOD level) or the computed LOD level.</remarks>
        /// <param name="node">node to be checked for visibility</param>
        /// <param name="cameraNode">camera node from which the visibility test is done</param>
        /// <param name="checkOcclusion">specifies if occlusion info from previous frame should be taken into account</param>
        /// <param name="calcLod">specifies if LOD level should be computed</param>
        /// <returns>computed LOD level or -1 if node is not visible</returns>
        public static int checkNodeVisibility(int node, int cameraNode, bool checkOcclusion, bool calcLod)
        {
            return NativeMethodsEngine.checkNodeVisibility(node, cameraNode, checkOcclusion, calcLod);
        }

        // Group specific
        /// <summary>
        /// This function creates a new Group node and attaches it to the specified parent node.
        /// </summary>
        /// <param name="parent">handle to parent node to which the new node will be attached</param>
        /// <param name="name">name of the node</param>
        /// <returns>handle to the created node or 0 in case of failure</returns>
        public static int addGroupNode(int parent, string name)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);

            return (int)NativeMethodsEngine.addGroupNode(parent, name);
        }


        // Model specific
        /// <summary>
        /// This function creates a new Model node and attaches it to the specified parent node.
        /// </summary>
        /// <param name="parent">handle to parent node to which the new node will be attached</param>
        /// <param name="name">name of the node</param>
        /// <param name="geoRes">Geometry resource used by Model node</param>
        /// <returns>handle to the created node or 0 in case of failure</returns>
        public static int addModelNode(int parent, string name, int geoRes)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);

            return (int)NativeMethodsEngine.addModelNode(parent, name, geoRes);
        }

        /// <summary>
        /// This function is used to setup the specified animation stage (channel) of the specified Model node.
        /// </summary>
        /// <remarks>
        /// The function operates on Model nodes but accepts also Group nodes in which case the call is passed recursively to the Model child nodes.
        /// The function is used for animation blending. There is a fixed number of stages (by default 16) on which different animations can be played. 
        /// The animation mask determines which child-nodes of the model (joints or meshes) are affected by the specified animation on the stage to be configured. 
        /// If the mask is an empty string, the animation affects all nodes. Otherwise the mask can contain several node names separated by the two character sequence. 
        /// When a mask is specified, the initial state of all nodes is 'not affected by animation'. For every node in the mask the function recurses down the (skeleton-) 
        /// hierarchy starting at the currently processed node in the mask and inverts the state of the considered nodes. This makes it possible to do complex animation mixing.
        /// A simpler way to do animation mixing is using additive animations. If a stage is configured to be additive the engine calculates the difference between the current 
        /// frame and the first frame in the animation and adds this delta to the current transformation of the joints or meshes.
        /// </remarks>
        /// <param name="node">handle to the node to be modified</param>
        /// <param name="stage">index of the animation stage to be configured</param>
        /// <param name="res">handle to Animation resource</param>
        /// <param name="animMask">mask defining which nodes will be affected by animation</param>
        /// <param name="additive">flag indicating whether stage is additive</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool setupModelAnimStage(int node, int stage, int res, string animMask, bool additive)
        {
            if (animMask == null) throw new ArgumentNullException("animMask", Resources.StringNullExceptionString);

            return NativeMethodsEngine.setupModelAnimStage(node, stage, res, animMask, additive);
        }

        /// <summary>
        /// This function sets the current animation time and weight for a specified stage of the specified model.</summary>
        /// <remarks>
        /// The time corresponds to the frames of the animation and the animation is looped if the time is higher than the maximum number of frames in the Animation resource. 
        /// The weight is used for animation blending and determines how much influence the stage has compared to the other active stages. 
        /// When the sum of the weights of all stages is more than one, the animations on the lower stages get priority. 
        /// The function operates on Model nodes but accepts also Group nodes in which case the call is passed recursively to the Model child nodes.
        /// </remarks>
        /// <param name="node">handle to the node to be modified</param>
        /// <param name="stage">index of the animation stage to be modified</param>
        /// <param name="time">new animation time</param>
        /// <param name="weight">new animation weight</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool setModelAnimParams(int node, int stage, float time, float weight)
        {
            return NativeMethodsEngine.setModelAnimParams(node, stage, time, weight);
        }

        /// <summary>
        /// This function sets the weight of a specified morph target. If the target parameter is an empty string the weight of all morph targets in the specified Model node is modified. The function operates on Model nodes but accepts also Group nodes in which case the call is passed recursively to the Model child nodes. If the specified morph target is not found the function returns false.
        /// </summary>
        /// <param name="node">handle to the node to be modified</param>
        /// <param name="target">name of morph target</param>
        /// <param name="weight">new weight for morph target</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool setModelMorpher(int node, string target, float weight)
        {
            if (target == null) throw new ArgumentNullException("target", Resources.StringNullExceptionString);

            return NativeMethodsEngine.setModelMorpher(node, target, weight);
        }

        // Mesh specific
        /// <summary>
        /// This function creates a new Mesh node and attaches it to the specified parent node.
        /// </summary>
        /// <param name="parent">handle to parent node to which the new node will be attached</param>
        /// <param name="name">name of the node</param>
        /// <param name="matRes">Material resource used by Mesh node</param>
        /// <param name="batchStart">first vertex index in Geometry resource of parent Model node</param>
        /// <param name="batchCount">number of vertex indices in Geometry resource of parent Model node</param>
        /// <param name="vertRStart">minimum vertex array index contained in Geometry resource indices of parent Model node</param>
        /// <param name="vertREnd">maximum vertex array index contained in Geometry resource indices of parent Model node</param>
        /// <returns>handle to the created node or 0 in case of failure</returns>
        public static int addMeshNode(int parent, string name, int matRes, int batchStart, int batchCount, int vertRStart, int vertREnd)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);
            return (int)NativeMethodsEngine.addMeshNode(parent, name, matRes, batchStart, batchCount, vertRStart, vertREnd);
        }

   
        // Joint specific
        /// <summary>
        /// This function creates a new Joint node and attaches it to the specified parent node.
        /// </summary>
        /// <param name="parent">handle to parent node to which the new node will be attached</param>
        /// <param name="name">name of the node</param>
        /// <param name="jointIndex">index of joint in Geometry resource of parent Model node</param>
        /// <returns>handle to the created node or 0 in case of failure</returns>
        public static int addJointNode(int parent, string name, int jointIndex)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);

            return (int)NativeMethodsEngine.addJointNode(parent, name, jointIndex);
        }

        // Light specific
        /// <summary>
        /// This function creates a new Light node and attaches it to the specified parent node. The direction vector of the untransformed light node is pointing along the the negative z-axis. The specified material resource can define uniforms and projective textures. Furthermore it can contain a shader for doing lighting calculations if deferred shading is used. If no material is required the parameter can be zero. The context names define which shader contexts are used when rendering shadow maps or doing light calculations for forward rendering configurations.
        /// </summary>
        /// <param name="parent">handle to parent node to which the new node will be attached</param>
        /// <param name="name">name of the node</param>
        /// <param name="materialRes">material resource for light configuration or 0 if not used</param>
        /// <param name="lightingContext">name of the shader context used for doing light calculations</param>
        /// <param name="shadowContext">name of the shader context used for doing shadow map rendering</param>
        /// <returns>handle to the created node or 0 in case of failure</returns>
        public static int addLightNode(int parent, string name, int materialRes, string lightingContext, string shadowContext)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);

            return (int)NativeMethodsEngine.addLightNode(parent, name, materialRes, lightingContext, shadowContext);
        }


        /// <summary>
        /// This function sets the lighting and shadow shader contexts of the specified light source. 
        /// The contexts define which shader code is used when doing lighting calculations or rendering the shadow map. 
        /// The function works on a Light node or a Group node in which case the call is recursively passed to all child Light nodes.
        /// </summary>
        /// <param name="node">handle to the node to be modified</param>
        /// <param name="lightingContext">name of the shader context used for performing lighting calculations</param>
        /// <param name="shadowContext"><name of the shader context used for rendering shadow maps/param>
        /// <returns>true in case of success otherwise false</returns>
        public static  bool setLightContexts( int node, string lightingContext, string shadowContext )
        {
            if (lightingContext == null) throw new ArgumentNullException("lightingContext", Resources.StringNullExceptionString);
            if (shadowContext == null) throw new ArgumentNullException("shadowContext", Resources.StringNullExceptionString);

            return NativeMethodsEngine.setLightContexts( node, lightingContext, shadowContext);
        }


        // Camera specific
        /// <summary>
        /// This function creates a new Camera node and attaches it to the specified parent node.
        /// </summary>
        /// <param name="parent">handle to parent node to which the new node will be attached</param>
        /// <param name="name">name of the node</param>
        /// <returns>handle to the created node or 0 in case of failure</returns>
        public static int addCameraNode(int parent, string name, int pipelineRes)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);

            return (int)NativeMethodsEngine.addCameraNode(parent, name, pipelineRes);
        }
        


        /// <summary>
        /// This function calculates the view frustum planes of the specified camera node using the specified view parameters.
        /// </summary>
        /// <param name="node">handle to the Camera node which will be modified</param>
        /// <param name="fov">field of view (FOV) in degrees</param>
        /// <param name="aspect">aspect ratio</param>
        /// <param name="nearDist">distance of near clipping plane</param>
        /// <param name="farDist">distance of far clipping plane</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool setupCameraView(int node, float fov, float aspect, float nearDist, float farDist)
        {
            return NativeMethodsEngine.setupCameraView(node, fov, aspect, nearDist, farDist);
        }

        /// <summary>
        /// This function gets the camera projection matrix used for bringing the geometry to
        /// screen space and copies it to the specified array.
        /// </summary>
        /// <param name="node">handle to Camera node</param>
        /// <param name="projMat">pointer to float array with 16 elements</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool getCameraProjectionMatrix(int node, float[] projMat)
        {
            if (projMat.Length != 16) throw new ArgumentOutOfRangeException("projMat", Resources.MatrixOutOfRangeExceptionString);

            return NativeMethodsEngine.getCameraProjectionMatrix(node, projMat);
        }



        // Emitter specific
        /// <summary>
        /// This function creates a new Emitter node and attaches it to the specified parent node.
        /// </summary>
        /// <param name="parent">handle to parent node to which the new node will be attached</param>
        /// <param name="name">name of the node</param>
        /// <param name="matRes">handle to material resource used for rendering</param>
        /// <param name="effectRes">handle to effect resource used for configuring particle properties</param>
        /// <param name="maxParticleCount">maximal number of particles living at the same time</param>
        /// <param name="respawnCount">number of times a single particle is recreated after dying (-1 for infinite)</param>
        /// <returns>handle to the created node or 0 in case of failure</returns>
        public static int addEmitterNode(int parent, string name, int matRes, int effectRes, int maxParticleCount, int respawnCount)
        {
            if (name == null) throw new ArgumentNullException("name", Resources.StringNullExceptionString);

            return (int)NativeMethodsEngine.addEmitterNode(parent, name, matRes, effectRes, maxParticleCount, respawnCount);
        }


        /// <summary>
        /// This function advances the simulation time of a particle system and continues the particle simulation with timeDelta being the time elapsed since the last call of this function.
        /// </summary>
        /// <param name="node">handle to the Emitter node which will be modified</param>
        /// <param name="timeDelta">time delta in seconds</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool advanceEmitterTime(int node, float timeDelta)
        {
            return NativeMethodsEngine.advanceEmitterTime(node, timeDelta);
        }

        /// <summary>
        /// Checks if an Emitter node is still alive.
        /// </summary>
        /// This function checks if a particle system is still active and has living particles or
        /// will spawn new particles. The specified node must be an Emitter node. The function can be
        /// used to check when a not infinitely running emitter for an effect like an explosion can be
        /// removed from the scene.
        /// <param name="emitterNode">emitterNode	- handle to the Emitter node which is checked</param>
        /// <returns>true if Emitter will no more emit any particles, otherwise or in case of failure false</returns>
        public static bool hasEmitterFinished(int emitterNode)
        {
            return NativeMethodsEngine.hasEmitterFinished(emitterNode);
        }
    }

    /// <summary>
    /// This Exception is thrown in case you don't use the correct Horde3D engine library version.
    /// </summary>
    [Serializable]
    public class LibraryIncompatibleException : Exception
    {
        public LibraryIncompatibleException()
        {
        }

        public LibraryIncompatibleException(string message)
            : base(message)
        {
        }

        public LibraryIncompatibleException(string message, Exception e)
            : base(message,e)
        {
        }

        protected LibraryIncompatibleException(SerializationInfo info, StreamingContext context) : base(info,context)
        {
        }

    }

}