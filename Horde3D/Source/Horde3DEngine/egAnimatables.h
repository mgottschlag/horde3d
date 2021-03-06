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

#ifndef _egAnimatables_H_
#define _egAnimatables_H_

#include "egPrerequisites.h"
#include "egScene.h"
#include "egMaterial.h"
#include "utMath.h"

class ModelNode;


// The following class is not real SceneNode, rather some sort of interface
class AnimatableSceneNode : public SceneNode
{
protected:

	ModelNode   *_parentModel;
	bool        _ignoreAnim;

public:

	AnimatableSceneNode( const SceneNodeTpl &tpl ) :
		SceneNode( tpl ), _parentModel( 0x0 ), _ignoreAnim( false )
	{
	}

	friend class SceneNode;
	friend class ModelNode;
};


// =================================================================================================
// Mesh Node
// =================================================================================================

struct MeshNodeParams
{
	enum List
	{
		MaterialRes = 300,
		BatchStart,
		BatchCount,
		VertRStart,
		VertREnd,
		LodLevel
	};
};

// =================================================================================================

struct MeshNodeTpl : public SceneNodeTpl
{
	PMaterialResource  matRes;
	uint32             batchStart, batchCount;
	uint32             vertRStart, vertREnd;
	uint32             lodLevel;

	MeshNodeTpl( const std::string &name, MaterialResource *materialRes, uint32 batchStart,
	             uint32 batchCount, uint32 vertRStart, uint32 vertREnd ) :
		SceneNodeTpl( SceneNodeTypes::Mesh, name ), matRes( materialRes ), batchStart( batchStart ),
		batchCount( batchCount ), vertRStart( vertRStart ), vertREnd( vertREnd ), lodLevel( 0 )
	{
	}
};

// =================================================================================================

class MeshNode : public AnimatableSceneNode
{
protected:

	PMaterialResource   _materialRes;
	uint32              _batchStart, _batchCount;
	uint32              _vertRStart, _vertREnd;
	uint32              _lodLevel;
	
	BoundingBox         _localBBox;
	bool                _bBoxDirty;

	MeshNode( const MeshNodeTpl &meshTpl );

public:

	static SceneNodeTpl *parsingFunc( std::map< std::string, std::string > &attribs );
	static SceneNode *factoryFunc( const SceneNodeTpl &nodeTpl );

	void markBBoxesDirty();
	BoundingBox *getLocalBBox() { return &_localBBox; }
	bool canAttach( SceneNode &parent );
	int getParami( int param );
	bool setParami( int param, int value );
	bool checkIntersection( const Vec3f &rayOrig, const Vec3f &rayDir, Vec3f &intsPos ) const;

	void onAttach( SceneNode &parentNode );
	void onDetach( SceneNode &parentNode );
	void onPreUpdate();

	MaterialResource *getMaterialRes() { return _materialRes; }
	uint32 getBatchStart() { return _batchStart; }
	uint32 getBatchCount() { return _batchCount; }
	uint32 getVertRStart() { return _vertRStart; }
	uint32 getVertREnd() { return _vertREnd; }
	uint32 getLodLevel() { return _lodLevel; }

	friend class ModelNode;
};


// =================================================================================================
// Joint Node
// =================================================================================================

struct JointNodeParams
{
	enum List
	{
		JointIndex = 400,
	};
};

// =================================================================================================

struct JointNodeTpl : public SceneNodeTpl
{
	uint32  jointIndex;

	JointNodeTpl( const std::string &name, uint32 jointIndex ) :
		SceneNodeTpl( SceneNodeTypes::Joint, name ), jointIndex( jointIndex )
	{
	}
};

// =================================================================================================

class JointNode : public AnimatableSceneNode
{
protected:

	uint32    _jointIndex;
	Matrix4f  _relModelMat;  // Transformation relative to parent model

	JointNode( const JointNodeTpl &jointTpl );

public:
	
	static SceneNodeTpl *parsingFunc( std::map< std::string, std::string > &attribs );
	static SceneNode *factoryFunc( const SceneNodeTpl &nodeTpl );
	
	bool canAttach( SceneNode &parent );
	int getParami( int param );

	void onPostUpdate();
	void onAttach( SceneNode &parentNode );
	void onDetach( SceneNode &parentNode );

	friend class ModelNode;
};

#endif // _egAnimatables_H_
