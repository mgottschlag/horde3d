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


namespace Horde3DNET.PlatformInvoke
{
    /// <summary>
    /// The PInvoke class provides access to important Win32 API functions.
    /// </summary>
    public static class PInvoke
    {
        /// <summary>
        /// The GetDC function retrieves a handle to a display device context (DC) for the client area of a specified window or for the entire screen.
        /// For more information visit: http://msdn2.microsoft.com/en-us/library/ms533241.aspx
        /// </summary>
        /// <param name="hWnd">handle to window</param>
        /// <returns>Handle to the device context. Otherwise null</returns>
        public static IntPtr GetDC(IntPtr hWnd)
        {
            return NativeMethods.GetDC(hWnd);
        }

        /// <summary>
        /// Retrieves the specified system metric or system configuration setting.
        /// For more information visit: http://msdn2.microsoft.com/en-us/library/ms724385.aspx
        /// </summary>
        /// <param name="nIndex">system metric or configuration setting to be retrieved</param>
        /// <returns>System metric or configuration setting</returns>
        public static int GetSystemMetrics(int nIndex)
        {
            return NativeMethods.GetSystemMetrics(nIndex);
        }

        /// <summary>
        /// The GetWindowDC function retrieves the device context (DC) for the entire window.
        /// For more information visit: http://msdn2.microsoft.com/en-us/library/ms534830.aspx
        /// </summary>
        /// <param name="hWnd">handle to window</param>
        /// <returns>Handle to a device context for the specified window. Otherwise null</returns>
        public static IntPtr GetWindowDC(int hWnd)
        {
            return NativeMethods.GetWindowDC(hWnd);
        }

        /// <summary>
        /// The ReleaseDC function releases a device context (DC).
        /// For more information visit: http://msdn2.microsoft.com/en-us/library/ms533251.aspx
        /// </summary>
        /// <param name="hWnd">handle to window</param>
        /// <param name="hDc">handle to device context</param>
        /// <returns>If the DC was released, the return value is 1. Otherwise 0.</returns>
        public static int ReleaseDC(IntPtr hWnd, IntPtr hDc)
        {
            return NativeMethods.ReleaseDC(hWnd, hDc);
        }

        /// <summary>
        /// The GetAsyncKeyState function determines whether a key is up or down at the time the function is called, and whether the key was pressed after a previous call to GetAsyncKeyState.
        /// For more information visit: http://msdn2.microsoft.com/en-us/library/ms646293.aspx
        /// </summary>
        /// <param name="vKey">Specifies one of 256 possible virtual-key codes</param>
        /// <returns>Key status information</returns>
        public static short GetAsyncKeyState(int vKey)
        {
            return NativeMethods.GetAsyncKeyState(vKey);
        }
    }

    /// <summary>
    /// Separates native methods from managed code.
    /// </summary>
    internal static class NativeMethods
    {
        [DllImport("user32.dll", EntryPoint = "GetDC")]
        public static extern IntPtr GetDC(IntPtr hWnd);

        [DllImport("user32.dll", EntryPoint = "GetSystemMetrics")]
        public static extern int GetSystemMetrics(int nIndex);

        [DllImport("user32.dll", EntryPoint = "GetWindowDC")]
        public static extern IntPtr GetWindowDC(int hWnd);

        [DllImport("user32.dll", EntryPoint = "ReleaseDC")]
        public static extern int ReleaseDC(IntPtr hWnd, IntPtr hDc);

        [DllImport("user32.dll", EntryPoint = "GetAsyncKeyState", SetLastError = true)]
        public static extern short GetAsyncKeyState(int vKey);
    }
}
