// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2008 Nicolas Schulz
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

#ifndef _egCamera_H_
#define _egCamera_H_

#include "egPrerequisites.h"
#include "egScene.h"
#include "egPipeline.h"
#include "egTextures.h"


// =================================================================================================
// Camera Node
// =================================================================================================

struct CameraNodeParams
{
	enum List
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
	};
};

// =================================================================================================

struct CameraNodeTpl : public SceneNodeTpl
{
	PPipelineResource   pipeRes;
	PTexture2DResource  outputTex;
	float               leftPlane, rightPlane;
	float               bottomPlane, topPlane;
	float               nearPlane, farPlane;
	int                 outputBufferIndex;
	bool                orthographic;
	bool                occlusionCulling;

	CameraNodeTpl( const std::string &name, PipelineResource *pipelineRes ) :
		SceneNodeTpl( SceneNodeTypes::Camera, name ), pipeRes( pipelineRes ),
		outputTex( 0x0 ), outputBufferIndex( 0 ),
		// Default params: fov=45, aspect=4/3
		leftPlane( -0.055228457f ), rightPlane( 0.055228457f ), bottomPlane( -0.041421354f ),
		topPlane( 0.041421354f ), nearPlane( 0.1f ), farPlane( 1000.0f ), orthographic( false ),
		occlusionCulling( false )
	{
	}
};

// =================================================================================================

class CameraNode : public SceneNode
{
private:

	PPipelineResource   _pipelineRes;
	PTexture2DResource  _outputTex;
	Matrix4f            _viewMat, _projMat;
	Frustum             _frustum;
	Vec3f               _absPos;
	float               _frustLeft, _frustRight, _frustBottom, _frustTop;
	float               _frustNear, _frustFar;
	int                 _outputBufferIndex;
	int                 _occSet;
	bool                _orthographic;  // Perspective or orthographic frustum?

	void onPostUpdate();

	CameraNode( const CameraNodeTpl &cameraTpl );

public:
	
	~CameraNode();

	static SceneNodeTpl *parsingFunc( std::map< std::string, std::string > &attribs );
	static SceneNode *factoryFunc( const SceneNodeTpl &nodeTpl );
	
	float getParamf( int param );
	bool setParamf( int param, float value );
	int getParami( int param );
	bool setParami( int param, int value );

	void setupViewParams( float fov, float aspect, float near, float far );

	const Frustum &getFrustum() { return _frustum; }
	const Matrix4f &getViewMat() { return _viewMat; }
	const Matrix4f &getProjMat() { return _projMat; }
	const Vec3f &getAbsPos() { return _absPos; }

	friend class SceneManager;
	friend class Renderer;
};

#endif // _egCamera_H_
