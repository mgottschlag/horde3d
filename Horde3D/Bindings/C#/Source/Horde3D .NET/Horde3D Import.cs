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
    internal static class NativeMethodsEngine
    {
        private const string ENGINE_DLL = "Horde3D.dll";

        // added (Horde3D 1.0)        

        // --- Basic funtions ---
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern IntPtr getVersionString();        

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool checkExtension(string extensionName);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool init();

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void release();

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void resize(int x, int y, int width, int height);
        
        //horde3d 1.0
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static extern bool render(int node);
        /////

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void clear();


        // --- General functions ---
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern IntPtr getMessage(out int level, out float time);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern float getOption(Horde3D.EngineOptions param);


        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool setOption(int param, float value);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern float getStat(int param, [MarshalAs(UnmanagedType.U1)]bool reset);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void showOverlay(float x_ll, float y_ll, float u_ll, float v_ll,
						                      float x_lr, float y_lr, float u_lr, float v_lr,
						                      float x_ur, float y_ur, float u_ur, float v_ur,
						                      float x_ul, float y_ul, float u_ul, float v_ul,
						                      int layer, int material);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void clearOverlays();

        // --- Resource functions ---
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern Horde3D.ResourceTypes getResourceType(int res);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern IntPtr getResourceName(int res);
        
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
	    internal static extern int findResource(Horde3D.ResourceTypes type, string name);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int addResource(Horde3D.ResourceTypes type, string name, int flags);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int cloneResource(int sourceRes, string name);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern int removeResource(int res);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static extern bool isResourceLoaded(int res);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool loadResource(string name, IntPtr data, int size);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool unloadResource(int res);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern int getResourceParami(int res, int param);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool setResourceParami(int res, int param, int value);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern float getResourceParamf(int res, int param);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool setResourceParamf(int res, int param, float value);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern IntPtr getResourceParamstr(int res, int param);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern bool setResourceParamstr(int res, int param, string value);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern IntPtr getResourceData(int res, int param);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool updateResourceData(int res, int param, IntPtr data, int size);
        
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int queryUnloadedResource(int index);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern void releaseUnusedResources();

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int createTexture2D(string name, int flags, int width, int height, [MarshalAs(UnmanagedType.U1)]bool renderable);

        // --- Shader specific ---
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool setShaderPreambles( string vertPreamble, string fragPreamble );

        // --- Material specific ---
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
	    internal static extern bool setMaterialUniform(int matRes, string name, float a, float b, float c, float d);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool setPipelineStageActivation(int pipelineRes, string stageName, [MarshalAs(UnmanagedType.U1)]bool enabled);

        //DLL bool getPipelineRenderTargetData( ResHandle pipelineRes, const char *targetName,
        //                              int bufIndex, int *width, int *height, int *compCount,
        //                              float *dataBuffer, int bufferSize );

        // --- Scene graph functions ---
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern Horde3D.SceneNodeTypes getNodeType(int node);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern IntPtr getNodeName(int node);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool setNodeName(int node, string name);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int getNodeParent(int node);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool setNodeParent(int node, int parent);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int getNodeChild(int parent, int index);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool setNodeAttachmentString(int node, string attachmentData);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern IntPtr getNodeAttachmentString(int node);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int addNodes(int parent, int res);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type
        internal static extern bool removeNode(int node);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool setNodeActivation(int node, [MarshalAs(UnmanagedType.U1)]bool active);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type
        internal static extern bool checkNodeTransformFlag(int node, [MarshalAs(UnmanagedType.U1)]bool reset);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool getNodeTransform(int node, out float px, out float py, out float pz,
                                out float rx, out float ry, out float rz, out float sx, out float sy, out float sz);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool setNodeTransform(int node, float px, float py, float pz,
                                float rx, float ry, float rz, float sx, float sy, float sz);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool getNodeTransformMatrices(int node, out IntPtr relMat, out IntPtr absMat);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool setNodeTransformMatrix(int node, float[] mat4x4);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]        
        internal static extern float getNodeParamf(int node, int param);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool setNodeParamf(int node, int param, float value);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int getNodeParami(int node, int param);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool setNodeParami(int node, int param, int value);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern IntPtr getNodeParamstr(int node, int param);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static extern bool setNodeParamstr(int node, int param, string value);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool getNodeAABB(int node, out float minX, out float minY, out float minZ, out float maxX, out float maxY, out float maxZ);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int findNodes(int node, string name, int type);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int getNodeFindResult(int index);
        
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int castRay(int node, float ox, float oy, float oz, float dx, float dy, float dz, int numNearest);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool getCastRayResult(int index, int node, out float distance, float[] intersection);

        // Group specific
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int addGroupNode(int parent, string name);

        // Model specific
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int addModelNode(int parent, string name, int geoRes);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool setupModelAnimStage(int node, int stage, int res,
                                      string animMask, [MarshalAs(UnmanagedType.U1)]bool additive);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool setModelAnimParams(int node, int stage, float time, float weight);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool setModelMorpher(int node, string target, float weight);

        // Mesh specific
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int addMeshNode(int parent, string name, int matRes, 
								    int batchStart, int batchCount,
							    int vertRStart, int vertREnd );
      
        // Joint specific
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int addJointNode(int parent, string name, int jointIndex);

        // Light specific
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int addLightNode(int parent, string name, int materialRes,
                                     string lightingContext, string shadowContext);

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool setLightContexts( int node, string lightingContext, string shadowContext );


        // Camera specific
        //horde3d 1.0
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int addCameraNode(int parent, string name, int pipelineRes);
        /////

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return : MarshalAs(UnmanagedType.U1)] // represents C++ bool type 
        internal static extern bool setupCameraView(int node, float fov, float aspect, float nearDist, float farDist);
            
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return : MarshalAs(UnmanagedType.U1)] // represents C++ bool type 
        internal static extern bool calcCameraProjectionMatrix(int node, float[] projMat);


	    // Emitter specific
        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        internal static extern int addEmitterNode( int parent, string name,
								       int matRes, int effectRes,
								       int maxParticleCount, int respawnCount );

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
	    internal static extern bool advanceEmitterTime( int node, float timeDelta );

        [DllImport(ENGINE_DLL), SuppressUnmanagedCodeSecurity]
        [return: MarshalAs(UnmanagedType.U1)]   // represents C++ bool type 
        internal static extern bool hasEmitterFinished(int emitterNode);
    }
}