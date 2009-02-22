// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
// --------------------------------------
// Copyright (C) 2006-2009 Nicolas Schulz
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

#ifndef _egScene_H_
#define _egScene_H_

#include "egPrerequisites.h"
#include "utMath.h"
#include "egPrimitives.h"
#include "egResource.h"
#include "egPipeline.h"
#include <map>


struct SceneNodeTpl;
class CameraNode;
class SceneGraphResource;

const int RootNode = 1;


// =================================================================================================
// Scene Node
// =================================================================================================

struct SceneNodeTypes
{
	enum List
	{
		Undefined = 0,
		Group,
		Model,
		Mesh,
		Joint,
		Light,
		Camera,
		Emitter
	};
};

struct SceneNodeParams
{
	enum List
	{
		Name = 1,
		AttachmentString
	};
};

// =================================================================================================

struct SceneNodeTpl
{
	int                            type;
	std::string                    name;
	Vec3f                          trans, rot, scale;
	std::string                    attachmentString;
	std::vector< SceneNodeTpl * >  children;

	SceneNodeTpl( int type, const std::string &name ) :
		type( type ), name( name ), scale( Vec3f ( 1, 1, 1 ) )
	{
	}
	
	virtual ~SceneNodeTpl()
	{
		for( uint32 i = 0; i < children.size(); ++i ) delete children[i];
	}
};

// =================================================================================================

class SceneNode
{
protected:
	
	Matrix4f                    _relTrans, _absTrans;  // Transformation matrices
	SceneNode                   *_parent;  // Parent node
	int                         _type;
	NodeHandle                  _handle;
	uint32                      _sgHandle;  // Spatial graph handle
	bool                        _dirty;  // Does the node need to be updated?
	bool                        _transformed;
	bool                        _renderable;
	bool                        _active;

	BoundingBox                 _bBox;  // AABB in world space

	std::vector< SceneNode * >  _children;  // Child nodes
	std::string                 _name;
	std::string                 _attachment;  // User defined data
	
	void markChildrenDirty();

	virtual void onPreUpdate();	// Called before absolute transformation is updated
	virtual void onPostUpdate();	// Called after absolute transformation has been updated
	virtual void onFinishedUpdate();  // Called after children have been updated
	virtual void onAttach( SceneNode &parentNode );	// Called when node is attached to parent
	virtual void onDetach( SceneNode &parentNode );	// Called when node is detached from parent

public:

	float                       tmpSortValue;
	
	SceneNode( const SceneNodeTpl &tpl );
	virtual ~SceneNode();

	void setActivation( bool active ) { _active = active; }
	void getTransform( Vec3f &trans, Vec3f &rot, Vec3f &scale );	// Not virtual for performance
	void setTransform( Vec3f trans, Vec3f rot, Vec3f scale );	// Not virtual for performance
	void setTransform( const Matrix4f &mat );
	const void getTransMatrices( const float **relMat, const float **absMat );

	virtual float getParamf( int param );
	virtual bool setParamf( int param, float value );
	virtual int getParami( int param );
	virtual bool setParami( int param, int value );
	virtual const char *getParamstr( int param );
	virtual bool setParamstr( int param, const char* value );

	virtual uint32 calcLodLevel( const Vec3f &viewPoint );

	virtual BoundingBox *getLocalBBox() { return 0x0; }
	virtual bool canAttach( SceneNode &parent );
	void markDirty();
	bool update();
	virtual bool checkIntersection( const Vec3f &rayOrig, const Vec3f &rayDir, Vec3f &intsPos ) const;

	int getType() { return _type; };
	NodeHandle getHandle() { return _handle; }
	SceneNode *getParent() { return _parent; }
	const std::string &getName() { return _name; }
	std::vector< SceneNode * > &getChildren() { return _children; }
	Matrix4f &getRelTrans() { return _relTrans; }
	Matrix4f &getAbsTrans() { return _absTrans; }
	BoundingBox &getBBox() { return _bBox; }
	const std::string &getAttachmentString() { return _attachment; }
	void setAttachmentString( const char* attachmentData ) { _attachment = attachmentData; }
	bool checkTransformFlag( bool reset )
		{ bool b = _transformed; if( reset ) _transformed = false; return b; }

	friend class SceneManager;
	friend class SpatialGraph;
	friend class Renderer;
};


// =================================================================================================
// Group Node
// =================================================================================================

struct GroupNodeTpl : public SceneNodeTpl
{
	GroupNodeTpl( const std::string &name ) :
		SceneNodeTpl( SceneNodeTypes::Group, name )
	{
	}
};

// =================================================================================================

class GroupNode : public SceneNode
{
protected:

	GroupNode( const GroupNodeTpl &groupTpl );

public:

	static SceneNodeTpl *parsingFunc( std::map< std::string, std::string > &attribs );
	static SceneNode *factoryFunc( const SceneNodeTpl &nodeTpl );

	float getParamf( int param );
	bool setParamf( int param, float value );

	friend class Renderer;
	friend class SceneManager;
};


// =================================================================================================
// Spatial Graph
// =================================================================================================

struct RendQueueEntry
{
	SceneNode  *node;
	int        type;  // Type is stored explicitly for better cache efficiency when iterating over list

	RendQueueEntry() {}
	RendQueueEntry( int type, SceneNode *node ) : type( type ), node( node ) {}
};

class SpatialGraph
{
protected:
	std::vector< SceneNode * >     _nodes;		// Renderable nodes and lights
	std::vector< uint32 >          _freeList;
	std::vector< SceneNode * >     _lightQueue;
	std::vector< RendQueueEntry >  _renderableQueue;

	static bool frontToBackOrder( RendQueueEntry n1, RendQueueEntry n2 )
		{ return n1.node->tmpSortValue < n2.node->tmpSortValue; }
	static bool backToFrontOrder( RendQueueEntry n1, RendQueueEntry n2 )
		{ return n1.node->tmpSortValue > n2.node->tmpSortValue; }

public:
	SpatialGraph();
	
	void addNode( SceneNode &sceneNode );
	void removeNode( uint32 sgHandle );
	void updateNode( uint32 sgHandle );

	void updateQueues( const Frustum &frustum1, const Frustum *frustum2,
	                   RenderingOrder::List order, bool lightQueue, bool renderQueue );

	std::vector< SceneNode * > &getLightQueue() { return _lightQueue; }
	std::vector< RendQueueEntry > &getRenderableQueue() { return _renderableQueue; }
};


// =================================================================================================
// Scene Manager
// =================================================================================================

typedef SceneNodeTpl *(*NodeTypeParsingFunc)( std::map< std::string, std::string > &attribs );
typedef SceneNode *(*NodeTypeFactoryFunc)( const SceneNodeTpl &tpl );
typedef void (*NodeTypeRenderFunc)( const std::string &shaderContext, const std::string &theClass, bool debugView,
                                    const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order,
                                    int occSet );

struct NodeRegEntry
{
	std::string          typeString;
	NodeTypeParsingFunc  parsingFunc;
	NodeTypeFactoryFunc  factoryFunc;
	NodeTypeRenderFunc   renderFunc;
};

struct CastRayResult
{
	SceneNode  *node;
	float      distance;
	Vec3f      intersection;
};

// =================================================================================================

class SceneManager
{
protected:

	std::vector< SceneNode *>      _nodes;  // _nodes[0] is root node
	std::vector< uint32 >          _freeList;  // List of free slots
	std::vector< SceneNode * >     _findResults;
	std::vector< CastRayResult >   _castRayResults;
	SpatialGraph                   *_spatialGraph;

	std::map< int, NodeRegEntry >  _registry;  // Registry of node types

	Vec3f                          _rayOrigin;  // Don't put these values on the stack during recursive search
	Vec3f                          _rayDirection;  // Ditto
	int                            _rayNum;  // Ditto

	NodeHandle parseNode( SceneNodeTpl &tpl, SceneNode *parent );
	void removeNodeRec( SceneNode *node );

	void castRayInternal( SceneNode *node );
public:

	SceneManager();
	~SceneManager();

	void registerType( int type, const std::string &typeString, NodeTypeParsingFunc pf,
	                   NodeTypeFactoryFunc ff, NodeTypeRenderFunc rf );
	NodeRegEntry *findType( int type );
	NodeRegEntry *findType( const std::string &typeString );
	
	void updateNodes();
	void updateSpatialNode( uint32 sgHandle ) { _spatialGraph->updateNode( sgHandle ); }
	void updateQueues( const Frustum &frustum1, const Frustum *frustum2,
	                   RenderingOrder::List order, bool lightQueue, bool renderableQueue );
	
	NodeHandle addNode( SceneNode *node, SceneNode &parent );
	NodeHandle addNodes( SceneNode &parent, SceneGraphResource &sgRes );
	bool removeNode( NodeHandle handle );
	bool relocateNode( NodeHandle node, NodeHandle parent );
	
	int findNodes( SceneNode *startNode, const std::string &name, int type );
	void clearFindResults() { _findResults.resize( 0 ); }
	SceneNode *getFindResult( int index ) { return (unsigned)index < _findResults.size() ? _findResults[index] : 0x0; }
	
	int castRay( SceneNode *node, const Vec3f &rayOrig, const Vec3f &rayDir, int numNearest );
	bool getCastRayResult( int index, CastRayResult &crr );

	int checkNodeVisibility( SceneNode *node, CameraNode *cam, bool checkOcclusion, bool calcLod );

	SceneNode &getRootNode() { return *_nodes[0]; }
	SceneNode &getDefCamNode() { return *_nodes[1]; }
	std::vector< SceneNode * > &getLightQueue() { return _spatialGraph->getLightQueue(); }
	std::vector< RendQueueEntry > &getRenderableQueue() { return _spatialGraph->getRenderableQueue(); }
	
	SceneNode *resolveNodeHandle( NodeHandle handle )
		{ return (handle != 0 && (unsigned)(handle - 1) < _nodes.size()) ? _nodes[handle - 1] : 0x0; }

	friend class Renderer;
};

#endif // _egScene_H_
