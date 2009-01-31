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

#include "egScene.h"
#include "egModules.h"
#include "egSceneGraphRes.h"
#include "egModel.h"
#include "egMaterial.h"
#include "egLight.h"
#include "egCamera.h"
#include "egParticle.h"

#include "utDebug.h"

using namespace std;

// *************************************************************************************************
// Class SceneNode
// *************************************************************************************************

SceneNode::SceneNode( const SceneNodeTpl &tpl ) :
	_type( tpl.type ), _parent( 0x0 ), _handle( 0 ), _sgHandle( 0 ),
	_dirty( true ), _transformed( true ), _renderable( false ), _active( true ),
	_name( tpl.name ), _attachment( tpl.attachmentString )
{
	setTransform( tpl.trans, tpl.rot, tpl.scale );
}


SceneNode::~SceneNode()
{
}


void SceneNode::getTransform( Vec3f &trans, Vec3f &rot, Vec3f &scale )
{
	if( _dirty ) Modules::sceneMan().updateNodes();
	
	_relTrans.decompose( trans, rot, scale );
	rot.x = radToDeg( rot.x );
	rot.y = radToDeg( rot.y );
	rot.z = radToDeg( rot.z );
}


void SceneNode::setTransform( Vec3f trans, Vec3f rot, Vec3f scale )
{
	// Hack to avoid making setTransform virtual
	if( _type == SceneNodeTypes::Joint || _type == SceneNodeTypes::Mesh )
	{
		((AnimatableSceneNode *)this)->_ignoreAnim = true;
	}
	
	_relTrans = Matrix4f::ScaleMat( scale.x, scale.y, scale.z );
	_relTrans.rotate( degToRad( rot.x ), degToRad( rot.y ), degToRad( rot.z ) );
	_relTrans.translate( trans.x, trans.y, trans.z );
	
	markDirty();
}


void SceneNode::setTransform( const Matrix4f &mat )
{
	// Hack to avoid making setTransform virtual
	if( _type == SceneNodeTypes::Joint || _type == SceneNodeTypes::Mesh )
	{
		((AnimatableSceneNode *)this)->_ignoreAnim = true;
	}
	
	_relTrans = mat;
	
	markDirty();
}


const void SceneNode::getTransMatrices( const float **relMat, const float **absMat )
{
	if( relMat != 0x0 )
	{
		if( _dirty ) Modules::sceneMan().updateNodes();
		*relMat = &_relTrans.x[0];
	}
	
	if( absMat != 0x0 )
	{
		if( _dirty ) Modules::sceneMan().updateNodes();
		*absMat = &_absTrans.x[0];
	}
}


float SceneNode::getParamf( int /*param*/ )
{
	return Math::NaN;
}


bool SceneNode::setParamf( int /*param*/, float /*value*/ )
{
	return false;
}


int SceneNode::getParami( int /*param*/ )
{
	return Math::MinInt32;
}


bool SceneNode::setParami( int /*param*/, int /*value*/ )
{
	return false;
}


const char *SceneNode::getParamstr( int param )
{
	switch( param )
	{
	case SceneNodeParams::Name:
		return _name.c_str();
	case SceneNodeParams::AttachmentString:
		return _attachment.c_str();
	default:
		return "";
	}
}


bool SceneNode::setParamstr( int param, const char *value )
{
	switch( param )
	{
	case SceneNodeParams::Name:
		_name = value;
		return true;
	case SceneNodeParams::AttachmentString:
		_attachment = value;
		return true;
	default:
		return false;
	}
}


bool SceneNode::canAttach( SceneNode &/*parent*/ )
{
	return true;
}


void SceneNode::markChildrenDirty()
{	
	for( uint32 i = 0, s = (uint32)_children.size(); i < s; ++i )
	{
		if( !_children[i]->_dirty )
		{	
			_children[i]->_dirty = true;
			_children[i]->_transformed = true;
			_children[i]->markChildrenDirty();
		}
	}
}


void SceneNode::markDirty()
{
	_dirty = true;
	_transformed = true;
	
	SceneNode *node = _parent;
	while( node != 0x0 )
	{
		node->_dirty = true;
		node = node->_parent;
	}

	markChildrenDirty();
}


bool SceneNode::update()
{
	if( !_dirty ) return false;
	
	bool bBoxChanged = false;
	
	onPreUpdate();
	
	// Calculate absolute matrix
	if( _parent != 0x0 )
		//_absTrans = _parent->_absTrans * _relTrans;
		_absTrans.fastMult( _parent->_absTrans, _relTrans );
	else
		_absTrans = _relTrans;
	
	// If there is a local bounding box, transform it to world space
	BoundingBox *locBBox = getLocalBBox();
	if( locBBox != 0x0 )
	{
		_bBox = *locBBox;
		_bBox.transform( _absTrans );
		bBoxChanged = true;
	}
	
	Modules::sceneMan().updateSpatialNode( _sgHandle );

	onPostUpdate();

	_dirty = false;

	// Visit children
	for( uint32 i = 0, s = (uint32)_children.size(); i < s; ++i )
	{
		bBoxChanged |= _children[i]->update();
	}	

	// Recalculate bounding box if necessary
	if( bBoxChanged )
	{
		if( locBBox == 0x0 )
		{	
			bBoxChanged = false;
			_bBox.clear();
		}

		for( uint32 i = 0, s = (uint32)_children.size(); i < s; ++i )
		{
			bBoxChanged |= _bBox.makeUnion( _children[i]->_bBox ); 
		}
	}

	onFinishedUpdate();
	
	return bBoxChanged;
}


bool SceneNode::checkIntersection( const Vec3f &/*rayOrig*/, const Vec3f &/*rayDir*/, Vec3f &/*intsPos*/ ) const
{
	return false;
}


void SceneNode::onPreUpdate()
{
}


void SceneNode::onPostUpdate()
{
}


void SceneNode::onFinishedUpdate()
{
}


void SceneNode::onAttach( SceneNode &/*parentNode*/ )
{
}


void SceneNode::onDetach( SceneNode &/*parentNode*/ )
{
}



// *************************************************************************************************
// Class GroupNode
// *************************************************************************************************

GroupNode::GroupNode( const GroupNodeTpl &groupTpl ) :
	SceneNode( groupTpl )
{
}


SceneNodeTpl *GroupNode::parsingFunc( map< string, string > &attribs )
{
	map< string, string >::iterator itr;
	GroupNodeTpl *groupTpl = new GroupNodeTpl( "" );
	
	return groupTpl;
}


SceneNode *GroupNode::factoryFunc( const SceneNodeTpl &nodeTpl )
{
	if( nodeTpl.type != SceneNodeTypes::Group ) return 0x0;
	
	return new GroupNode( *(GroupNodeTpl *)&nodeTpl );
}


float GroupNode::getParamf( int param )
{
	return SceneNode::getParamf( param );
}


bool GroupNode::setParamf( int param, float value )
{
	return SceneNode::setParamf( param, value );
}


// =================================================================================================
// Class SpatialGraph
// =================================================================================================

SpatialGraph::SpatialGraph()
{
	_lightQueue.reserve( 20 );
	_renderableQueue.reserve( 500 );
}


void SpatialGraph::addNode( SceneNode &sceneNode )
{	
	if( !sceneNode._renderable && sceneNode._type != SceneNodeTypes::Light ) return;
	
	if( !_freeList.empty() )
	{
		uint32 slot = _freeList.back();
		ASSERT( _nodes[slot] == 0x0 );
		_freeList.pop_back();

		sceneNode._sgHandle = slot + 1;
		_nodes[slot] = &sceneNode;
	}
	else
	{
		sceneNode._sgHandle = (uint32)_nodes.size() + 1;
		_nodes.push_back( &sceneNode );
	}
}


void SpatialGraph::removeNode( uint32 sgHandle )
{
	if( sgHandle == 0 || _nodes[sgHandle - 1] == 0x0 ) return;

	// Reset queues
	_lightQueue.resize( 0 );
	_renderableQueue.resize( 0 );
	
	_nodes[sgHandle - 1]->_sgHandle = 0;
	_nodes[sgHandle - 1] = 0x0;
	_freeList.push_back( sgHandle - 1 );
}


void SpatialGraph::updateNode( uint32 sgHandle )
{
	// Since the spatial graph is just a flat list of objects,
	// there is nothing to do at the moment
}


void SpatialGraph::updateQueues( const Frustum &frustum1, const Frustum *frustum2,
	                             RenderingOrder::List order, bool lightQueue, bool renderQueue )
{
	Modules::sceneMan().updateNodes();
	
	// Clear without affecting capacity
	if( lightQueue ) _lightQueue.resize( 0 );
	if( renderQueue ) _renderableQueue.resize( 0 );

	// Culling
	for( size_t i = 0, s = _nodes.size(); i < s; ++i )
	{
		SceneNode *node = _nodes[i];
		if( !node || !node->_active ) continue;

		if( renderQueue && node->_renderable )
		{
			if( !frustum1.cullBox( node->_bBox ) &&
				(frustum2 == 0x0 || !frustum2->cullBox( node->_bBox )) )
			{
				if( order != RenderingOrder::None )
				{
					node->tmpSortValue = nearestDistToAABB( frustum1.getOrigin(),
						node->_bBox.getMinCoords(), node->_bBox.getMaxCoords() );
				}
				_renderableQueue.push_back( RendQueueEntry( node->_type, node ) );	
			}
		}
		else if( lightQueue && node->_type == SceneNodeTypes::Light )
		{
			_lightQueue.push_back( node );
		}
	}

	// Sort
	if( order == RenderingOrder::FrontToBack )
		std::sort( _renderableQueue.begin(), _renderableQueue.end(), SpatialGraph::frontToBackOrder );
	else if( order == RenderingOrder::BackToFront )
		std::sort( _renderableQueue.begin(), _renderableQueue.end(), SpatialGraph::backToFrontOrder );
}


// *************************************************************************************************
// Class SceneManager
// *************************************************************************************************

SceneManager::SceneManager()
{
	SceneNode *rootNode = GroupNode::factoryFunc( GroupNodeTpl( "RootNode" ) );
	rootNode->_handle = RootNode;
	_nodes.push_back( rootNode );

	_spatialGraph = new SpatialGraph();
}


SceneManager::~SceneManager()
{
	delete _spatialGraph;

	for( uint32 i = 0; i < _nodes.size(); ++i )
	{
		delete _nodes[i]; _nodes[i] = 0x0;
	}
}


void SceneManager::registerType( int type, const string &typeString, NodeTypeParsingFunc pf,
								 NodeTypeFactoryFunc ff, NodeTypeRenderFunc rf )
{
	NodeRegEntry entry;
	entry.typeString = typeString;
	entry.parsingFunc = pf;
	entry.factoryFunc = ff;
	entry.renderFunc = rf;
	_registry[type] = entry;
}


NodeRegEntry *SceneManager::findType( int type )
{
	map< int, NodeRegEntry >::iterator itr = _registry.find( type );
	
	if( itr != _registry.end() ) return &itr->second;
	else return 0x0;
}


NodeRegEntry *SceneManager::findType( const string &typeString )
{
	map< int, NodeRegEntry >::iterator itr = _registry.begin();

	while( itr != _registry.end() )
	{
		if( itr->second.typeString == typeString ) return &itr->second;

		++itr;
	}
	
	return 0x0;
}


void SceneManager::updateNodes()
{
	getRootNode().update();
}


void SceneManager::updateQueues( const Frustum &frustum1, const Frustum *frustum2,
								 RenderingOrder::List order, bool lightQueue, bool renderableQueue )
{
	_spatialGraph->updateQueues( frustum1, frustum2, order, lightQueue, renderableQueue );
}


NodeHandle SceneManager::parseNode( SceneNodeTpl &tpl, SceneNode *parent )
{
	if( parent == 0x0 ) return 0;
	
	SceneNode *sn = 0x0;

	if( tpl.type == 0 )
	{
		// Reference node
		NodeHandle handle = parseNode( *((ReferenceNodeTpl *)&tpl)->sgRes->getRootNode(), parent );
		sn = Modules::sceneMan().resolveNodeHandle( handle );
		if( sn != 0x0 )
		{	
			sn->_name = tpl.name;
			sn-> setTransform( tpl.trans, tpl.rot, tpl.scale );
			sn->_attachment = tpl.attachmentString;
		}
	}
	else
	{
		map< int, NodeRegEntry >::iterator itr = _registry.find( tpl.type );
		if( itr != _registry.end() ) sn = (*itr->second.factoryFunc)( tpl );
		if( sn != 0x0 ) addNode( sn, *parent );
	}

	if( sn == 0x0 ) return 0;

	// Parse children
	for( uint32 i = 0; i < tpl.children.size(); ++i )
	{
		parseNode( *tpl.children[i], sn );
	}

	return sn->getHandle();
}


NodeHandle SceneManager::addNode( SceneNode *node, SceneNode &parent )
{
	if( node == 0x0 ) return 0;
	
	// Check if node can be attached to parent
	if( !node->canAttach( parent ) )
	{
		Modules::log().writeDebugInfo( "Can't attach node '%s' to parent '%s'", node->_name.c_str(), parent._name.c_str() );
		delete node; node = 0x0;
		return 0;
	}
	
	node->_parent = &parent;
	
	// Attach to parent
	parent._children.push_back( node );

	// Raise event
	node->onAttach( parent );

	// Mark tree as dirty
	node->markDirty();

	// Register node in spatial graph
	_spatialGraph->addNode( *node );
	
	// Insert node in free slot
	if( !_freeList.empty() )
	{
		uint32 slot = _freeList.back();
		ASSERT( _nodes[slot] == 0x0 );
		_freeList.pop_back();

		node->_handle = slot + 1;
		_nodes[slot] = node;
		return slot + 1;
	}
	else
	{
		_nodes.push_back( node );
		node->_handle = (NodeHandle)_nodes.size();
		return node->_handle;
	}
}


NodeHandle SceneManager::addNodes( SceneNode &parent, SceneGraphResource &sgRes )
{
	// Parse root node
	return parseNode( *sgRes.getRootNode(), &parent );
}


void SceneManager::removeNodeRec( SceneNode *node )
{
	NodeHandle handle = node->_handle;
	
	// Raise event
	if( handle != RootNode ) node->onDetach( *node->_parent );

	// Remove children
	for( uint32 i = 0; i < node->_children.size(); ++i )
	{
		removeNodeRec( node->_children[i] );
	}
	
	// Delete node
	if( handle != RootNode )
	{
		_spatialGraph->removeNode( node->_sgHandle );
		delete _nodes[handle - 1]; _nodes[handle - 1] = 0x0;
		_freeList.push_back( handle - 1 );
	}
}


bool SceneManager::removeNode( NodeHandle handle )
{
	SceneNode *sn = resolveNodeHandle( handle );
	if( sn == 0x0 )		// Call allowed for rootnode
	{
		Modules::log().writeDebugInfo( "Invalid node handle %i in removeNode", handle );
		return false;
	}

	SceneNode *parent = sn->_parent;

	removeNodeRec( sn );
	
	// Remove node from parent
	if( handle != RootNode )
	{
		// Find child
		for( uint32 i = 0; i < parent->_children.size(); ++i )
		{
			if( parent->_children[i] == sn )
			{
				parent->_children.erase( parent->_children.begin() + i );
				break;
			}
		}
	}
	else
	{
		sn->_children.clear();
	}
	
	// Mark dirty
	if( parent != 0x0 ) parent->markDirty();
	
	return true;
}


bool SceneManager::relocateNode( NodeHandle node, NodeHandle parent )
{
	if( node == RootNode ) return 0;
	
	SceneNode *sn = resolveNodeHandle( node );
	if ( sn == 0x0 )
	{	
		Modules::log().writeDebugInfo( "Invalid node handle %i in relocateNode", node );
		return 0;
	}
	SceneNode *snp = resolveNodeHandle( parent );
	if ( snp == 0x0 )
	{	
		Modules::log().writeDebugInfo( "Invalid parent node handle %i in relocateNode", parent );
		return 0;
	}

	if( !sn->canAttach( *snp ) )
	{	
		Modules::log().writeDebugInfo( "Can't attach node %i to parent %i in relocateNode", node, parent );
		return 0;
	}
	
	// Detach from old parent
	sn->onDetach( *sn->_parent );
	for( uint32 i = 0; i < sn->_parent->_children.size(); ++i )
	{
		if( sn->_parent->_children[i] == sn )
		{
			sn->_parent->_children.erase( sn->_parent->_children.begin() + i );
			break;
		}
	}

	// Attach to new parent
	snp->_children.push_back( sn );
	sn->_parent = snp;
	sn->onAttach( *snp );
	
	snp->markDirty();
	sn->_parent->markDirty();
	
	return true;
}


int SceneManager::findNodes( SceneNode *startNode, const string &name, int type )
{
	int count = 0;
	
	if( type == SceneNodeTypes::Undefined || startNode->_type == type )
	{
		if( name == "" || startNode->_name == name )
		{
			_findResults.push_back( startNode );
			++count;
		}
	}

	for( uint32 i = 0; i < startNode->_children.size(); ++i )
	{
		count += findNodes( startNode->_children[i], name, type );
	}

	return count;
}


void SceneManager::castRayInternal( SceneNode *node )
{
	if( !node->_active ) return;
	
	if( rayAABBIntersection( _rayOrigin, _rayDirection, node->_bBox.getMinCoords(), node->_bBox.getMaxCoords() ) )
	{
		Vec3f intsPos;
		if( node->checkIntersection( _rayOrigin, _rayDirection, intsPos ) )
		{
			float dist = (intsPos - _rayOrigin).length();

			CastRayResult crr;
			crr.node = node;
			crr.distance = dist;
			crr.intersection = intsPos;

			bool inserted = false;
			for( vector< CastRayResult >::iterator it = _castRayResults.begin(); it != _castRayResults.end(); ++it )
			{
				if( dist < it->distance )
				{
					_castRayResults.insert( it, crr );
					inserted = true;
					break;
				}
			}

			if( !inserted )
			{
				_castRayResults.push_back( crr );
			}

			if( _rayNum > 0 && (int) _castRayResults.size() > _rayNum )
			{
				_castRayResults.pop_back();
			}
		}

		for( size_t i = 0, s = node->_children.size(); i < s; ++i )
		{
			castRayInternal( node->_children[i] );
		}
	}
}


int SceneManager::castRay( SceneNode *node, const Vec3f &rayOrig, const Vec3f &rayDir, int numNearest )
{
	_castRayResults.resize(0); // empty results vector by resizing it to zero (does not release the memory to avoid reallocation)

	if( !node->_active ) return 0;

	_rayOrigin = rayOrig;
	_rayDirection = rayDir;
	_rayNum = numNearest;

	castRayInternal( node );

	return (int) _castRayResults.size();
}


bool SceneManager::getCastRayResult( int index, CastRayResult &crr )
{
	if( (uint32) index < _castRayResults.size() )
	{
		crr = _castRayResults[index];

		return true;
	}

	return false;
}


int SceneManager::checkNodeVisibility( SceneNode *node, CameraNode *cam, bool checkOcclusion, bool calcLod )
{
	// Note: This function is a bit hacky with all the hard-coded node types
	
	if( node->_dirty ) updateNodes();

	// Check occlusion
	if( checkOcclusion && cam->_occSet >= 0 )
	{
		if( node->getType() == SceneNodeTypes::Model )
		{
			if( Modules::renderer().getOccQueryResult( ((ModelNode *)node)->_occQueries[cam->_occSet] ) < 1 )
				return -1;
		}
		else if( node->getType() == SceneNodeTypes::Emitter )
		{
			if( Modules::renderer().getOccQueryResult( ((EmitterNode *)node)->_occQueries[cam->_occSet] ) < 1 )
				return -1;
		}
		else if( node->getType() == SceneNodeTypes::Light )
		{
			if( Modules::renderer().getOccQueryResult( ((LightNode *)node)->_occQueries[cam->_occSet] ) < 1 )
				return -1;
		}
	}
	
	// Frustum culling
	if( cam->getFrustum().cullBox( node->getBBox() ) )
	{
		return -1;
	}
	else if( node->getType() == SceneNodeTypes::Model )
	{
		// Calculate LOD level
		return ((ModelNode *)node)->calcLodLevel( cam->getAbsPos() );
	}
	else
	{
		return 0;
	}
}
