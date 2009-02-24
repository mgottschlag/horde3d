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

#include "egModel.h"
#include "egMaterial.h"
#include "egModules.h"

#include "utDebug.h"

using namespace std;

ModelNode::ModelNode( const ModelNodeTpl &modelTpl ) :
	SceneNode( modelTpl ), _geometryRes( modelTpl.geoRes ), _baseGeoRes( 0x0 ),
	_softwareSkinning( modelTpl.softwareSkinning ), _morpherUsed( false ), _morpherDirty( false ),
	_animDirty( false ), _nodeListDirty( false ), _skinningDirty( false ), _meshCount( 0 ),
	_lodDist1( modelTpl.lodDist1 ), _lodDist2( modelTpl.lodDist2 ), _lodDist3( modelTpl.lodDist3 ),
	_lodDist4( modelTpl.lodDist4 )
{
	_renderable = true;
	
	for( uint32 i = 0; i < MaxNumAnimStages; ++i ) _animStages[i] = 0x0;
	
	if( _geometryRes != 0x0 )
		setParami( ModelNodeParams::GeometryRes, _geometryRes->getHandle() );
}


ModelNode::~ModelNode()
{
	_geometryRes = 0x0; _baseGeoRes = 0x0;
	for( uint32 i = 0; i < _occQueries.size(); ++i )
	{
		if( _occQueries[i] != 0 )
			Modules::renderer().destroyOccQuery( _occQueries[i] );
	}

	for( uint32 i = 0; i < MaxNumAnimStages; ++i ) delete _animStages[i];
}


SceneNodeTpl *ModelNode::parsingFunc( map< string, string > &attribs )
{
	bool result = true;
	
	ModelNodeTpl *modelTpl = new ModelNodeTpl( "", 0x0 );
	
	map< string, string >::iterator itr = attribs.find( "geometry" );
	if( itr != attribs.end() )
	{
		uint32 res = Modules::resMan().addResource( ResourceTypes::Geometry, itr->second, 0, false );
		if( res != 0 )
			modelTpl->geoRes = (GeometryResource *)Modules::resMan().resolveResHandle( res );
	}
	else result = false;
	itr = attribs.find( "softwareSkinning" );
	if( itr != attribs.end() ) 
	{
		if ( _stricmp( itr->second.c_str(), "true" ) == 0 || _stricmp( itr->second.c_str(), "1" ) == 0 )
			modelTpl->softwareSkinning = true;
		else
			modelTpl->softwareSkinning = false;
	}

	itr = attribs.find( "lodDist1" );
	if( itr != attribs.end() ) modelTpl->lodDist1 = (float)atof( itr->second.c_str() );
	itr = attribs.find( "lodDist2" );
	if( itr != attribs.end() ) modelTpl->lodDist2 = (float)atof( itr->second.c_str() );
	itr = attribs.find( "lodDist3" );
	if( itr != attribs.end() ) modelTpl->lodDist3 = (float)atof( itr->second.c_str() );
	itr = attribs.find( "lodDist4" );
	if( itr != attribs.end() ) modelTpl->lodDist4 = (float)atof( itr->second.c_str() );

	if( !result )
	{
		delete modelTpl; modelTpl = 0x0;
	}
	
	return modelTpl;
}


SceneNode *ModelNode::factoryFunc( const SceneNodeTpl &nodeTpl )
{
	if( nodeTpl.type != SceneNodeTypes::Model ) return 0x0;

	return new ModelNode( *(ModelNodeTpl *)&nodeTpl );
}


void ModelNode::updateStageAnimations( uint32 stage, const string &startNode )
{
	AnimationResource *anim = _animStages[stage]->anim;
	if( anim == 0x0 )
	{	
		for( size_t i = 0, s = _nodeList.size(); i < s; ++i ) 
		{
			_nodeList[i].animEntities[stage] = 0x0;
		}
		
		return;
	}

	// Find animation resource entries for nodes
	for( size_t i = 0, s = _nodeList.size(); i < s; ++i )
	{
		bool includeNode = true;
		
		if( startNode != "" )
		{
			includeNode = false;
			
			SceneNode *node = _nodeList[i].node;
			while( node->getType() != SceneNodeTypes::Model )
			{
				if( node->getName() == startNode )
				{
					includeNode = true;
					break;
				}
				node = node->getParent();
			}
		}
		
		if( includeNode )
			_nodeList[i].animEntities[stage] = anim->findEntity( _nodeList[i].node->getParamstr( SceneNodeParams::Name ) );
		else
			_nodeList[i].animEntities[stage] = 0x0;
	}
}


void ModelNode::recreateNodeListRec( SceneNode *node, bool firstCall )
{
	if( node->getType() == SceneNodeTypes::Mesh )
	{
		++_meshCount;
		_nodeList.push_back( NodeListEntry( (AnimatableSceneNode *)node ) );
		if( _nodeList.size() > _meshCount )
			swap( _nodeList[_meshCount - 1], _nodeList.back() );
	}
	else if( node->getType() == SceneNodeTypes::Joint )
	{
		_nodeList.push_back( NodeListEntry( (AnimatableSceneNode *)node ) );
	}
	else if( !firstCall ) return;	// First node is the model

	// Children
	for( size_t i = 0, s = node->getChildren().size(); i < s; ++i )
	{
		recreateNodeListRec( node->getChildren()[i], false );
	}
}


void ModelNode::recreateNodeList()
{
	_meshCount = 0;
	_nodeList.resize( 0 );
	
	recreateNodeListRec( this, true );
	for( uint32 i = 0; i < MaxNumAnimStages; ++i )
	{
		if( _animStages[i] != 0x0 && _animStages[i]->anim != 0x0 )
		{
			updateStageAnimations( i, _animStages[i]->startNode );
		}
	}

	_nodeListDirty = false;
}


bool ModelNode::setupAnimStage( int stage, uint32 animRes, const string &startNode, bool additive )
{
	if( (unsigned)stage >= MaxNumAnimStages ) return false;

	Resource *res = Modules::resMan().resolveResHandle( animRes );
	
	// Resource ID 0 is allowed for erasing stage
	if( animRes != 0 )
	{
		if( res == 0x0 || res->getType() != ResourceTypes::Animation )
		{
			Modules::log().writeDebugInfo( "Invalid Animation resource for Model node %i", _handle );
			return false;
		}
	}
	
	AnimStage *curStage = _animStages[stage];
	if( animRes == 0 )
	{
		// Erase stage
		if( curStage != 0x0 )
		{
			delete _animStages[stage]; _animStages[stage] = 0x0;
		}
		
		markDirty();	// Mark scene node as dirty so that update function is called
		_animDirty = true;
		
		return true;
	}
	else if( curStage != 0x0 )
	{
		// Reset stage
		if( curStage->anim != 0x0 ) curStage->anim = 0x0;
	}
	else
	{
		// Create stage
		_animStages[stage] = new AnimStage();
		curStage = _animStages[stage];
	}
	
	curStage->startNode = startNode;
	curStage->additive = additive;
	curStage->anim = (AnimationResource *)res;

	updateStageAnimations( stage, startNode );

	return setAnimParams( stage, 0.0f, 1.0f );
}


bool ModelNode::setAnimParams( int stage, float time, float weight )
{
	if( (unsigned)stage > MaxNumAnimStages ) return false;

	AnimStage *curStage = _animStages[stage];
	if( curStage == 0x0 || curStage->anim == 0x0 ) return false;

	curStage->animTime = time;
	curStage->blendWeight = weight;

	// Reset ignore animation flag
	for( size_t i = 0, s = _nodeList.size(); i < s; ++i )
		_nodeList[i].node->_ignoreAnim = false;

	markDirty();	// Mark scene node as dirty so that update function is called
	_animDirty = true;
	
	return true;
}


bool ModelNode::setMorphParam( const string &targetName, float weight )
{
	if( _geometryRes == 0x0 || _morphers.empty() ) return false;

	bool result = false;
	_morpherDirty = true;
	_morpherUsed = false;

	// Set specified morph target or all targets if targetName == ""
	for( uint32 i = 0; i < _morphers.size(); ++i )
	{
		if( targetName == "" || _morphers[i].name == targetName )
		{
			_morphers[i].weight = weight;
			result = true;
		}

		if( _morphers[i].weight > 0 ) _morpherUsed = true;
	}

	markDirty();

	return result;
}


int ModelNode::getParami( int param )
{
	switch( param )
	{
	case ModelNodeParams::GeometryRes:
		return _geometryRes != 0x0 ? _geometryRes->_handle : 0;
	case ModelNodeParams::SoftwareSkinning:
		return _softwareSkinning ? 1 : 0;
	default:
		return SceneNode::getParami( param );
	}
}


void ModelNode::markMeshBBoxesDirty()
{
	for( size_t i = 0, s = _children.size(); i < s; ++i )
	{
		if( _children[i]->getType() == SceneNodeTypes::Mesh )
			((MeshNode *)_children[i])->markBBoxesDirty();
	}

	markDirty();
}


float ModelNode::getParamf( int param )
{
	switch( param )
	{
	case ModelNodeParams::LodDist1:
		return _lodDist1;
	case ModelNodeParams::LodDist2:
		return _lodDist2;
	case ModelNodeParams::LodDist3:
		return _lodDist3;
	case ModelNodeParams::LodDist4:
		return _lodDist4;
	default:
		return SceneNode::getParamf( param );
	}
}


bool ModelNode::setParamf( int param, float value )
{
	switch( param )
	{
	case ModelNodeParams::LodDist1:
		_lodDist1 = value;
		return true;
	case ModelNodeParams::LodDist2:
		_lodDist2 = value;
		return true;
	case ModelNodeParams::LodDist3:
		_lodDist3 = value;
		return true;
	case ModelNodeParams::LodDist4:
		_lodDist4 = value;
		return true;
	default:	
		return SceneNode::setParamf( param, value );
	}
}


bool ModelNode::setParami( int param, int value )
{
	Resource *res;
	
	switch( param )
	{
	case ModelNodeParams::GeometryRes:
		res = Modules::resMan().resolveResHandle( value );
		if( res == 0x0 || res->getType() != ResourceTypes::Geometry )
		{	
			Modules::log().writeDebugInfo( "Invalid Geometry resource for Model node %i", _handle );
			return false;
		}

		// Init joint data
		_skinMatRows.resize( ((GeometryResource *)res)->_joints.size() * 3 );
		for( uint32 i = 0; i < _skinMatRows.size() / 3; ++i )
		{
			_skinMatRows[i * 3 + 0] = Vec4f( 1, 0, 0, 0 );
			_skinMatRows[i * 3 + 1] = Vec4f( 0, 1, 0, 0 );
			_skinMatRows[i * 3 + 2] = Vec4f( 0, 0, 1, 0 );
		}

		// Copy morph targets
		_morphers.resize( ((GeometryResource *)res)->_morphTargets.size() );
		for( uint32 i = 0; i < _morphers.size(); ++i )
		{	
			Morpher &morpher = _morphers[i]; 
			
			morpher.name = ((GeometryResource *)res)->_morphTargets[i].name;
			morpher.index = i;
			morpher.weight = 0;
		}

		if( !_morphers.empty() || _softwareSkinning )
		{
			Resource *clonedRes =  Modules::resMan().resolveResHandle(
				Modules::resMan().cloneResource( res->getHandle(), "" ) );
			_geometryRes = (GeometryResource *)clonedRes;
			_baseGeoRes = (GeometryResource *)res;
		}
		else
		{
			_geometryRes = (GeometryResource *)res;
			_baseGeoRes = 0x0;
		}

		_skinningDirty = true;
		markMeshBBoxesDirty();
		return true;
	case ModelNodeParams::SoftwareSkinning:
		_softwareSkinning = (value != 0);
		if( _softwareSkinning )
		{	
			_skinningDirty = true;
			markMeshBBoxesDirty();
		}

		if( _softwareSkinning && _baseGeoRes == 0x0 && _geometryRes != 0x0 )
			// Create a local resource copy since it is not yet existing
			setParami( ModelNodeParams::GeometryRes, _geometryRes->getHandle() );
		else if( !_softwareSkinning && _morphers.empty() && _baseGeoRes != 0x0 )
			// Remove the local resource copy by removing reference
			setParami( ModelNodeParams::GeometryRes, _baseGeoRes->getHandle() );

		return true;

	default:
		return SceneNode::setParami( param, value );
	}
}


bool ModelNode::updateGeometry()
{
	_skinningDirty |= _morpherDirty;
	_skinningDirty &= _softwareSkinning;
	
	if( !_skinningDirty && !_morpherDirty ) return false;

	if( _baseGeoRes == 0x0 || _baseGeoRes->getVertData() == 0x0 ) return false;
	if( _geometryRes == 0x0 || _geometryRes->getVertData() == 0x0 ) return false;
	
	// Reset vertices to base data
	memcpy( _geometryRes->_vertData->memory, _baseGeoRes->_vertData->memory,
			_geometryRes->_vertCount * sizeof( Vec3f ) * 4 );

	if( _morpherUsed )
	{
		// Recalculate vertex positions for morph targets
		for( uint32 i = 0; i < _morphers.size(); ++i )
		{
			if( _morphers[i].weight > Math::Epsilon )
			{
				MorphTarget &mt = _geometryRes->_morphTargets[_morphers[i].index];
				float weight = _morphers[i].weight;
				
				for( uint32 j = 0; j < mt.diffs.size(); ++j )
				{
					MorphDiff &md = mt.diffs[j];
					VertexData &vd = *_geometryRes->getVertData();
					
					vd.positions[md.vertIndex] += md.posDiff * weight;
					vd.normals[md.vertIndex] += md.normDiff * weight;
					vd.tangents[md.vertIndex] += md.tanDiff * weight;
					vd.bitangents[md.vertIndex] += md.bitanDiff * weight;
				}
			}
		}
	}

	if( _skinningDirty )
	{
		// TODO: Optimize this routine

		for( uint32 i = 0, s = _geometryRes->getVertCount(); i < s; ++i )
		{
			Matrix4f mat, skinningMat;
			VertexData &vd = *_geometryRes->getVertData();
			
			for( uint32 j = 0; j < 4; ++j )
			{
				uint32 ind0 = (uint32)ftoi_r( vd.staticData[i].jointVec[j] ) * 3 + 0;
				uint32 ind1 = ind0 + 1, ind2 = ind0 + 2;
				
				mat.x[0] = _skinMatRows[ind0].x;
				mat.x[1] = _skinMatRows[ind1].x;
				mat.x[2] = _skinMatRows[ind2].x;
				mat.x[4] = _skinMatRows[ind0].y;
				mat.x[5] = _skinMatRows[ind1].y;
				mat.x[6] = _skinMatRows[ind2].y;
				mat.x[8] = _skinMatRows[ind0].z;
				mat.x[9] = _skinMatRows[ind1].z;
				mat.x[10] = _skinMatRows[ind2].z;
				mat.x[12] = _skinMatRows[ind0].w;
				mat.x[13] = _skinMatRows[ind1].w;
				mat.x[14] = _skinMatRows[ind2].w;

				if( j == 0) skinningMat = mat * vd.staticData[i].weightVec[j];
				else skinningMat += mat * vd.staticData[i].weightVec[j];
			}

			// Skin position
			vd.positions[i] = skinningMat * vd.positions[i];
			
			// Skin tangent space basis
			skinningMat.x[12] = 0;
			skinningMat.x[13] = 0;
			skinningMat.x[14] = 0;
			vd.normals[i] = skinningMat * vd.normals[i];
			vd.tangents[i] = skinningMat * vd.tangents[i];
			vd.bitangents[i] = skinningMat * vd.bitangents[i];
		}
	}
		
	// Renormalize tangent space basis
	for( uint32 i = 0, s = _geometryRes->getVertCount(); i < s; ++i )
	{
		VertexData &vd = *_geometryRes->getVertData();
		
		vd.normals[i].normalize();
		vd.tangents[i].normalize();
		vd.bitangents[i].normalize();
	}

	_morpherDirty = false;
	_skinningDirty = false;
	
	// Upload geometry
	_geometryRes->updateDynamicVertData();
	markMeshBBoxesDirty();
	return true;
}


uint32 ModelNode::calcLodLevel( const Vec3f &viewPoint )
{
	Vec3f pos( _absTrans.c[3][0], _absTrans.c[3][1], _absTrans.c[3][2] );
	float dist = (pos - viewPoint).length();
	uint32 curLod = 4;
	
	if( dist < _lodDist1 ) curLod = 0;
	else if( dist < _lodDist2 ) curLod = 1;
	else if( dist < _lodDist3 ) curLod = 2;
	else if( dist < _lodDist4 ) curLod = 3;

	return curLod;
}


void ModelNode::onPostUpdate()
{
	if( _nodeListDirty ) recreateNodeList();
	
	if( _animDirty )
	{	
		_skinningDirty = true;
		_animDirty = false;
		
		// Find active stages
		static std::vector< uint32 > activeStages;
		activeStages.resize( 0 );

		for( uint32 i = 0; i < MaxNumAnimStages; ++i )
		{
			if( _animStages[i] != 0x0 && _animStages[i]->anim != 0x0 )
				activeStages.push_back( i );
		}

		// Animate
		if( Modules::config().fastAnimation && activeStages.size() == 1 )
		{
			uint32 firstStage = activeStages[0];
			
			// Fast animation path
			for( size_t i = 0, s =_nodeList.size(); i < s; ++i )
			{
				// Ignore animation if node transformation was set manually
				if( _nodeList[i].node->_ignoreAnim )
				{
					_nodeList[i].node->_ignoreAnim = false;
					continue;
				}
				
				AnimResEntity *ae = _nodeList[i].animEntities[firstStage];
				if( ae != 0x0 && !ae->frames.empty() )
				{
					uint32 frame = (uint32)ftoi_t( _animStages[firstStage]->animTime ) % ae->frames.size();
					if( ae->frames.size() == 1 ) frame = 0;		// Animation compression
					_nodeList[i].node->getRelTrans() = ae->frames[frame].bakedTransMat;
				}
			}
		}
		else
		{
			Quaternion nodeRotQuat;
			Vec3f nodeTransVec, nodeScaleVec;
			
			for( size_t i = 0, s = _nodeList.size(); i < s; ++i )
			{
				// Ignore animation if node transformation was set manually
				if( _nodeList[i].node->_ignoreAnim )
				{
					_nodeList[i].node->_ignoreAnim = false;
					continue;
				}
				
				bool firstStage = true;
				float weightAccum = 0.0f;

				for( size_t j = 0, s = activeStages.size(); j < s; ++j )
				{
					uint32 stageIdx = activeStages[j];
					AnimStage &curStage = *_animStages[stageIdx];
				
					// Ignore stages with a blend weight near zero
					if( curStage.blendWeight < 0.0001f && !curStage.additive ) continue;

					if( _nodeList[i].animEntities[stageIdx] == 0x0 ) continue;
					uint32 numFrames = (uint32)_nodeList[i].animEntities[stageIdx]->frames.size();
					
					if( numFrames > 0 )
					{
						float weight = curStage.blendWeight;
						if( weightAccum + weight > 1.0f ) weight = 1.0f - weightAccum;

						// Fast animation with sampled frame data
						if( Modules::config().fastAnimation )
						{
							uint32 f0 = ftoi_t( curStage.animTime ) % numFrames;
							if( numFrames == 1 ) f0 = 0;	// Animation compression
							Frame &frame = _nodeList[i].animEntities[stageIdx]->frames[f0];
							
							if( firstStage )
							{
								// Ignore additive stages that are before a non-additive one
								if( !curStage.additive )
								{
									firstStage = false;
									weightAccum = curStage.blendWeight;
									nodeRotQuat = frame.rotQuat;
									nodeTransVec = frame.transVec;
									nodeScaleVec = frame.scaleVec;
								}
							}
							else
							{
								if( curStage.additive )
								{
									// Add the difference to the first frame of the animation
									Frame &firstFrame = _nodeList[i].animEntities[stageIdx]->frames[0];
									
									nodeRotQuat *= firstFrame.rotQuat.inverted() * frame.rotQuat;
									nodeTransVec += frame.transVec - firstFrame.transVec;
									nodeScaleVec.x *= 1 / firstFrame.scaleVec.x * frame.scaleVec.x;
									nodeScaleVec.y *= 1 / firstFrame.scaleVec.y * frame.scaleVec.y;
									nodeScaleVec.z *= 1 / firstFrame.scaleVec.z * frame.scaleVec.z;
								}
								else if( weightAccum < 1.0f )
								{
									float blend = weightAccum / (weightAccum + weight);
							
									nodeRotQuat = frame.rotQuat.slerp( nodeRotQuat, blend );
									nodeTransVec = frame.transVec.lerp( nodeTransVec, blend );
									nodeScaleVec = frame.scaleVec.lerp( nodeScaleVec, blend );

									weightAccum += curStage.blendWeight;
								}
							}
						}
						else	// Animation with inter-frame interpolation
						{
							uint32 f0 = ftoi_t( curStage.animTime );
							float amount = curStage.animTime - f0;
							f0 = f0 % numFrames;
							uint32 f1 = f0 + 1;
							if( f1 > numFrames - 1 ) f1 = numFrames - 1;

							if( numFrames == 1 ) f0 = f1 = 0;	// Animation compression
							
							Frame &frame0 = _nodeList[i].animEntities[stageIdx]->frames[f0];
							Frame &frame1 = _nodeList[i].animEntities[stageIdx]->frames[f1];
							
							// Inter-frame interpolation
							Vec3f transVec( frame0.transVec.lerp( frame1.transVec, amount ) );
							Vec3f scaleVec( frame0.scaleVec.lerp( frame1.scaleVec, amount ) );
							Quaternion rotQuat( frame0.rotQuat.slerp( frame1.rotQuat, amount ) );

							if( firstStage )
							{
								// Ignore additive stages that are before a non-additive one
								if( !curStage.additive )
								{
									firstStage = false;
									weightAccum = curStage.blendWeight;
									nodeRotQuat = rotQuat;
									nodeTransVec = transVec;
									nodeScaleVec = scaleVec;
								}
							}
							else
							{
								if( curStage.additive )
								{
									// Add the difference to the first frame of the animation
									Frame &firstFrame = _nodeList[i].animEntities[stageIdx]->frames[0];
									
									nodeRotQuat *= firstFrame.rotQuat.inverted() * rotQuat;
									nodeTransVec += transVec - firstFrame.transVec;
									nodeScaleVec.x *= 1 / firstFrame.scaleVec.x * scaleVec.x;
									nodeScaleVec.y *= 1 / firstFrame.scaleVec.y * scaleVec.y;
									nodeScaleVec.z *= 1 / firstFrame.scaleVec.z * scaleVec.z;
								}
								else if( weightAccum < 1.0f )
								{
									// Interpolate between animation and current state from previous animations
									float blend = weightAccum / (weightAccum + weight);
									
									nodeRotQuat = rotQuat.slerp( nodeRotQuat, blend );
									nodeTransVec = transVec.lerp( nodeTransVec, blend );
									nodeScaleVec = scaleVec.lerp( nodeScaleVec, blend );

									weightAccum += curStage.blendWeight;
								}
							}
						}
					}
				}

				// Build matrix from animation data
				Matrix4f mat( Math::NO_INIT );
				Matrix4f::fastMult43( mat, Matrix4f( nodeRotQuat ),
				                      Matrix4f::ScaleMat( nodeScaleVec.x, nodeScaleVec.y, nodeScaleVec.z ) );
				Matrix4f::fastMult43( _nodeList[i].node->_relTrans,
				                      Matrix4f::TransMat( nodeTransVec.x, nodeTransVec.y, nodeTransVec.z ), mat );
			}
		}

		// Mark transformed nodes as dirty
		markDirty();
	}
}


void ModelNode::onFinishedUpdate()
{
	// Update geometry for morphers or software skinning
	if( updateGeometry() )
	{
		update();	// Force update so that bounding boxes are adapted to skinned data
	}
}
