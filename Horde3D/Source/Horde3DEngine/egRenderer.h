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

#ifndef _egRenderer_H_
#define _egRenderer_H_

#include "egPrerequisites.h"
#include "egRendererBase.h"
#include "egPrimitives.h"
#include "egLight.h"
#include "egCamera.h"
#include "egModel.h"
#include <vector>
#include <algorithm>


class MaterialResource;
struct ShaderContext;

const uint32 ParticlesPerBatch = 64;	// Warning: The GPU must have enough registers

extern const char *vsDefColor;
extern const char *fsDefColor;
extern const char *vsOccBox;
extern const char *fsOccBox;	
	

// =================================================================================================
// Renderer
// =================================================================================================

struct EngineStats
{
	enum List
	{
		TriCount = 100,
		BatchCount,
		LightPassCount
	};
};

// =================================================================================================

struct Overlay
{
	PMaterialResource  materialRes;
	int                layer;
	float              x_tl, y_tl, u_tl, v_tl;  // Top left corner
	float              x_bl, y_bl, u_bl, v_bl;  // Bottom left corner
	float              x_br, y_br, u_br, v_br;  // Bottom right corner
	float              x_tr, y_tr, u_tr, v_tr;  // Top right corner
	float              colR, colG, colB, colA;  // Color


	Overlay() {}

	Overlay( const float &x_tl, const float &y_tl, const float &u_tl, const float &v_tl,
	         const float &x_bl, const float &y_bl, const float &u_bl, const float &v_bl,
	         const float &x_br, const float &y_br, const float &u_br, const float &v_br,
	         const float &x_tr, const float &y_tr, const float &u_tr, const float &v_tr,
			 const float &colR, const float &colG, const float &colB, const float &colA,
	         MaterialResource *matRes, int layer ) :
		x_tl( x_tl ), y_tl( y_tl ), u_tl( u_tl ), v_tl( v_tl ),
		x_bl( x_bl ), y_bl( y_bl ), u_bl( u_bl ), v_bl( v_bl ),
		x_br( x_br ), y_br( y_br ), u_br( u_br ), v_br( v_br ),
		x_tr( x_tr ), y_tr( y_tr ), u_tr( u_tr ), v_tr( v_tr ),
		colR( colR ), colG( colG ), colB( colB ), colA( colA ),
		materialRes( matRes ), layer( layer )
	{	
	}
};


struct ParticleVert
{
	float  x, y, z;  // Position
	float  u, v;  // Texture coordinates
	float  cornerIndex;  // Index of corner of quad (for up/right vectors)
	float  index;  // Index in property array

	ParticleVert()
	{
	}
	
	ParticleVert( float u, float v, float cornerIndex ):
		x( 0 ), y( 0 ), z( 0 ), u( u ), v( v ), cornerIndex( cornerIndex ), index( 0 )
	{
	}
};

// =================================================================================================

struct PipeSamplerBinding
{
	char          sampler[64];
	RenderBuffer  *rb;
	uint32        bufIndex;
};


class Renderer : public RendererBase
{
protected:

	std::vector< PipeSamplerBinding >  _pipeSamplerBindings;
	std::vector< char >                _occSets;  // Actually bool
	std::vector< Overlay >             _overlays;
	
	uint32                             _frameID;
	uint32                             _smFBO, _smTex;
	uint32                             _defShadowMap;
	uint32                             _particleVBO;
	MaterialResource                   *_curStageMatLink;
	CameraNode                         *_curCamera;
	LightNode                          *_curLight;
	MaterialResource                   *_curMatRes;
	ShaderCombination                  *_curShader;
	RenderTarget                       *_curRenderTarget;
	uint32                             _curUpdateStamp;
	
	float                              _splitPlanes[5];
	Matrix4f                           _lightMats[4];

	int                                _statTriCount, _statBatchCount, _statLightPassCount;


	static bool nodeFrontToBackOrder( NodeListEntry e1, NodeListEntry e2 )
		{ return ((MeshNode *)e1.node)->tmpSortValue < ((MeshNode *)e2.node)->tmpSortValue; }
	static bool nodeBackToFrontOrder( NodeListEntry e1, NodeListEntry e2 )
		{ return ((MeshNode *)e1.node)->tmpSortValue > ((MeshNode *)e2.node)->tmpSortValue; }
	static bool meshMaterialOrder( NodeListEntry e1, NodeListEntry e2 )
		{ return ((MeshNode *)e1.node)->getMaterialRes() < ((MeshNode *)e2.node)->getMaterialRes(); }
	
	void setupViewMatrices( CameraNode *cam );
	
	bool setMaterialRec( MaterialResource *materialRes, const std::string &shaderContext, ShaderResource *shaderRes );
	
	void setupShadowMap( bool noShadows );
	Matrix4f calcLightMat( const Frustum &frustum );
	void updateShadowMap();

	void drawOverlays( const std::string &shaderContext );

	void bindBuffer( RenderBuffer *rb, const std::string &sampler, uint32 bufIndex );
	void clear( bool depth, bool buf0, bool buf1, bool buf2, bool buf3, float r, float g, float b, float a );
	void drawFSQuad( Resource *matRes, const std::string &shaderContext );
	void drawGeometry( const std::string &shaderContext, const std::string &theClass,
	                   RenderingOrder::List order, int occSet );
	void drawLightGeometry( const std::string shaderContext, const std::string &theClass,
	                        bool noShadows, RenderingOrder::List order, int occSet );
	void drawLightShapes( const std::string shaderContext, bool noShadows, int occSet );
	
	void drawRenderables( const std::string &shaderContext, const std::string &theClass, bool debugView,
		const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order, int occSet );
	
	void renderDebugView();
	void finishRendering();

public:
	
	static ShaderCombination  defColorShader;
	static ShaderCombination  occShader;
	
	Renderer();
	~Renderer();
	
	void initStates();
	bool init();
	void resize( int x, int y, int width, int height );
	
	float getStat( int param, bool reset );
	void incStat( int param, float value );
	int registerOccSet();
	void unregisterOccSet( int occSet );

	bool uploadShader( const char *vertexShader, const char *fragmentShader, ShaderCombination &sc );
	void setShader( ShaderCombination *sc );
	bool setMaterial( MaterialResource *materialRes, const std::string &shaderContext );
	
	bool createShadowBuffer( uint32 width, uint32 height );
	void destroyShadowBuffer();

	void showOverlay( const Overlay &overlay );
	void clearOverlays();
	
	void drawAABB( const Vec3f &bbMin, const Vec3f &bbMax );
	void drawDebugAABB( const Vec3f &bbMin, const Vec3f &bbMax, bool saveStates );
	
	static void drawModels( const std::string &shaderContext, const std::string &theClass, bool debugView,
		const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order, int occSet );
	static void drawParticles( const std::string &shaderContext, const std::string &theClass, bool debugView,
		const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order, int occSet );

	bool render( CameraNode *camNode );

	uint32 getFrameID() { return _frameID; }
	uint32 getVPWidth() { return _vpWidth; }
	uint32 getVPHeight() { return _vpHeight; }
	ShaderCombination *getCurShader() { return _curShader; }
	CameraNode *getCurCamera() { return _curCamera; }
	uint32 getParticleVBO() { return _particleVBO; }
};

#endif // _egRenderer_H_
