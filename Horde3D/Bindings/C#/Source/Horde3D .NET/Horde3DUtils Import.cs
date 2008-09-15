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
using System.Runtime.InteropServices;
using System.Security;

namespace Horde3DNET
{
    /// <summary>
    /// Separates native methods from managed code.
    /// </summary>
    internal static class NativeMethodsUtils
    {
        public const string UTILS_DLL = "Horde3DUtils.dll";

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void freeMem(IntPtr ptr);

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool dumpMessages();

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool initOpenGL(int hDC);

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void releaseOpenGL();

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void swapBuffers();

        // Utilities
        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern IntPtr getResourcePath(Horde3D.ResourceTypes type);

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void setResourcePath(Horde3D.ResourceTypes type, string path);
             

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool loadResourcesFromDisk(string contentDir);


        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool createTGAImage(IntPtr pixels, uint width, uint height, uint bpp, out IntPtr outData, out uint outSize);

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void pickRay(int cameraNode, float nwx, float nwy,
            out float ox, out float oy, out float oz,
            out float dx, out float dy, out float dz);

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int pickNode(int node, float nwx, float nwy);

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void showText(string text, float x, float y, float size,
                                             int layer, uint fontMatRes);

        [DllImport(UTILS_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void showFrameStats(int fontMaterialRes, float curFPS);


    }
}
