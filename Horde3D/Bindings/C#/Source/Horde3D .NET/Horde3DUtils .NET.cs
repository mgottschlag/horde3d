// *************************************************************************************************
//
// Horde3D .NET wrapper
// ----------------------------------
// Copyright (C) 2007-2009 Martin Burkhard, Volker Wiendl
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
using System.Runtime.InteropServices;
using Horde3DNET.Properties;

namespace Horde3DNET
{
    public static class Horde3DUtils
    {
        /// <summary>        
        /// MaxStatMode  - Maximum stat mode number supported in showFrameStats
        /// </summary>
        public const int MaxStatMode = 2;

        // Utilities functions
        /// <summary>
        /// FreeMem is not supported. The purpose is to free memory allocated by the Horde3D library.
        /// </summary>
        public static void freeMem(IntPtr ptr)
        {
            NativeMethodsUtils.freeMem(ptr);
        }
        
        /// <summary>
        /// This utility function pops all messages from the message queue and writes them to a HTML formated log file 'EngineLog.html'.
        /// </summary>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool dumpMessages()
        {
            return NativeMethodsUtils.dumpMessages();
        }

        /// <summary>
        /// This utility function initializes an OpenGL rendering context in a specified window component.
        /// </summary>
        /// <param name="hDC">handle to device context for which OpenGL context shall be created</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool initOpenGL(int hDC)
        {
            return NativeMethodsUtils.initOpenGL(hDC);
        }

        /// <summary>
        /// This utility function destroys the previously created OpenGL rendering context.
        /// </summary>
        public static void releaseOpenGL()
        {
            NativeMethodsUtils.releaseOpenGL();
        }

        /// <summary>
        /// This utility function displays the image rendered to the previously initialized OpenGL context on the screen by copying the OpenGL backbuffer to the window front buffer.
        /// </summary>
        public static void swapBuffers()
        {
            NativeMethodsUtils.swapBuffers();
        }

        /// <summary>
        /// This function returns the search path of a specified resource type.
        /// </summary>
        /// <param name="type">type of resource</param>
        /// <returns>the search path string</returns>
        public static string getResourcePath(Horde3D.ResourceTypes type)
        {
            IntPtr ptr = NativeMethodsUtils.getResourcePath(type);
            return Marshal.PtrToStringAnsi(ptr);
        }

        /// <summary>
        /// This function sets the search path for a specified resource type. 
        /// Whenever a new resource is added, the specified path is concatenated to the name of the created resource.
        /// </summary>
        /// <param name="type">type of resource</param>
        /// <param name="path">path where the resources can be found (without slash or backslash at the end)</param>
        public static void setResourcePath(Horde3D.ResourceTypes type, string path)
        {
            if (path == null) throw new ArgumentNullException("path", Resources.StringNullExceptionString);

            NativeMethodsUtils.setResourcePath(type, path);
        }

        /// <summary>
        /// This utility function loads previously added and still unloaded resources from a specified directory on a data drive. 
        /// All resource names are directly converted to filenames when being loaded.
        /// </summary>
        /// <param name="contenDir">directory where data is located on the drive</param>
        /// <returns>false if at least one resource could not be loaded, otherwise true</returns>
        public static bool loadResourcesFromDisk(string contenDir)
        {
            if (contenDir == null) throw new ArgumentNullException("contenDir", Resources.StringNullExceptionString);

            return NativeMethodsUtils.loadResourcesFromDisk(contenDir);
        }

        /// <summary>
        /// This utility function allocates memory for the pointer outData and creates a TGA image from the specified pixel data. 
        /// The dimensions of the image have to be specified as well as the bit depth of the pixel data. 
        /// The created TGA-image-data can be used as Texture2D or TexureCube resource in the engine.
        /// </summary>
        /// <remarks>
        /// The memory allocated by this routine will be freed automatically.
        /// </remarks>
        /// <param name="pixels">pointer to pixel source data from which TGA-image-data is constructed; memory layout: pixel with position (x, y) in image (origin of image is upper left corner) has memory location (y * width + x) * (bpp / 8) in pixels array</param>
        /// <param name="width">image width of source data</param>
        /// <param name="height">image height of source data</param>
        /// <param name="bpp">color bit depth of source data (Valid: 24, 32)</param>
        /// <param name="outData">the created TGA data</param>
        /// <param name="outSize">size of TGA data</param>
        /// <returns>true in case of success, otherwise false</returns>
        public static bool createTGAImage(byte[] pixels, int width, int height, int bpp, out byte[] outData, out int outSize)
        {
            IntPtr pixelPtr, dataPtr;
            uint size;

            // Allocate data to store pixel content
            pixelPtr = Marshal.AllocHGlobal(pixels.Length);

            // Copy pixel content to memory location
            Marshal.Copy(pixels, 0, pixelPtr, pixels.Length);

            // create TGA Image 
            bool result = NativeMethodsUtils.createTGAImage(pixelPtr, (uint)width, (uint)height, (uint)bpp, out dataPtr, out size);

            // free allocated memory
            Marshal.FreeHGlobal(pixelPtr);

            // set size of TGA data
            outSize = (int)size;

            // create new array for TGA data
            outData = new byte[outSize];

            // copy TGA data from memory into data array
            Marshal.Copy(dataPtr, outData, 0, outSize);

            // free allocated memory for TGA data
            //NativeMethodsUtils.freeMem(dataPtr);

            return result;
        }

        /// <summary>
        /// Calculates the ray originating at the specified camera and window coordinates
        /// </summary>
        /// This utility function takes normalized window coordinates (ranging from 0 to 1 with the
        /// origin being the bottom left corner of the window) and returns ray origin and direction
        /// given camera. The function is especially useful for selecting objects by clicking
        /// on them.
        /// <param name="cameraNode">camera used for picking</param>
        /// <param name="nwx">normalized window coordinates</param>
        /// <param name="nwy">normalized window coordinates</param>
        /// <param name="ox">calculated ray origin</param>
        /// <param name="oy">calculated ray origin</param>
        /// <param name="oz">calculated ray origin</param>
        /// <param name="dx">calculated ray direction</param>
        /// <param name="dy">calculated ray direction</param>
        /// <param name="dz">calculated ray direction</param>
        public static void pickRay( 
            int cameraNode, float nwx, float nwy, 
            out float ox, out float oy, out float oz,
            out float dx, out float dy, out float dz)
        {
            NativeMethodsUtils.pickRay(cameraNode, nwx, nwy, out ox, out oy, out oz, out dx, out dy, out dz);
        }


        /// <summary>
        /// This utility function takes normalized window coordinates (ranging from 0 to 1 with the origin being the bottom left corner of the window) and returns the scene node which is visible at that location. The function is especially useful for selecting objects by clicking on them. Currently picking is only working for Meshes.
        /// </summary>
        /// <param name="nwx">normalized window x coordinate</param>
        /// <param name="nwy">normalized window y coordinate</param>
        /// <returns>handle of picked node or 0 if no node was hit</returns>
        public static int pickNode( int node, float nwx, float nwy)
        {
            return NativeMethodsUtils.pickNode(node, nwx, nwy);
        }

        /// <summary>
        /// This utility function uses overlays to display a text string at a specified position on the screen. 
        /// </summary>
        /// <remarks>
        /// The font texture of the specified font material has to be a regular 16x16 grid containing all ASCII characters in row-major order. 
        /// The layer corresponds to the layer parameter of overlays.
        /// </remarks>
        /// <param name="text">text string to be displayed</param>
        /// <param name="x">x position of the lower left corner of the first character; for more details on coordinate system see overlay documentation</param>
        /// <param name="y">y position of the lower left corner of the first character; for more details on coordinate system see overlay documentation</param>
        /// <param name="size">size factor of the font</param>
        /// <param name="colR">red part of font color</param>
        /// <param name="colG">green part of font color</param>
        /// <param name="colB">blue part of font color</param>
        /// <param name="fontMatRes">font material resource used for rendering</param>
        /// <param name="layer">layer index of the font overlays (values: 0-7)</param>        
        public static void showText(string text, float x, float y, float size,
                                    float colR, float colG, float colB,
                                    int fontMatRes, int layer)
        {
            if (text == null) throw new ArgumentNullException("text", Resources.StringNullExceptionString);
            if (layer < 0) throw new ArgumentOutOfRangeException("layer", Resources.UIntOutOfRangeExceptionString);
            if (fontMatRes < 0) throw new ArgumentOutOfRangeException("fontMatRes", Resources.UIntOutOfRangeExceptionString);

            NativeMethodsUtils.showText(text, x, y, size, colR, colG, colB, fontMatRes, layer);
        }

        /// <summary>
        /// This utility function displays an info box with statistics for the current frame on the screen.
		/// Since the statistic counters are reset after the call, the function should be called exactly once
		/// per frame to obtain correct values.
        /// </summary>
        /// <param name="fontMaterialRes">font material resource used for drawing text</param>
        /// <param name="panelMaterialRes">material resource used for drawing info box</param>
        /// <param name="mode">display mode, specifying which data is shown (<= MaxStatMode)</param>
        public static void showFrameStats(int fontMaterialRes, int panelMaterialRes, int mode)
        {
            NativeMethodsUtils.showFrameStats(fontMaterialRes, panelMaterialRes, mode);
        }
    }
}
