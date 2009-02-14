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

#ifndef _converter_H_
#define _converter_H_

#include "daeMain.h"
#include "utMath.h"


struct Joint;

struct Vertex
{
	Vec3f  storedPos, pos;
	Vec3f  storedNormal, normal, tangent, bitangent;
	Vec3f  texCoords[4];
	Joint  *joints[4];
	float  weights[4];	
	int    daePosIndex;


	Vertex()
	{
		joints[0] = 0x0; joints[1] = 0x0; joints[2] = 0x0; joints[3] = 0x0;
		weights[0] = 1; weights[1] = 0; weights[2] = 0; weights[3] = 0;
	}
};


struct TriGroup
{
	unsigned int  first, count;
	unsigned int  vertRStart, vertREnd;
	std::string   matName;

	unsigned int                 numPosIndices;
	std::vector< unsigned int >  *posIndexToVertices;

	TriGroup() : posIndexToVertices( 0x0 )
	{
	}
};


struct SceneNode
{
	bool                        typeJoint;
	char                        name[256];
	Matrix4f                    matRel, matAbs;
	DaeNode                     *daeNode;
	DaeInstance                 *daeInstance;
	SceneNode                   *parent;
	std::vector< SceneNode * >  children;

	// Animation
	std::vector< Matrix4f >     frames;  // Relative transformation for every frame

	SceneNode()
	{
		daeNode = 0x0;
		daeInstance = 0x0;
		parent = 0x0;
	}
};


struct Mesh : public SceneNode
{
	std::vector< TriGroup >  triGroups;
	unsigned int             lodLevel;
	
	Mesh()
	{
		typeJoint = false;
		parent = 0x0;
		lodLevel = 0;
	}
};


struct Joint : public SceneNode
{
	unsigned int  index;
	Matrix4f      invBindMat;
	bool          used;

	// Temporary
	Matrix4f      daeInvBindMat;

	Joint()
	{
		typeJoint = true;
		used = false;
	}
};


struct MorphDiff
{
	unsigned int  vertIndex;
	Vec3f         posDiff;
	Vec3f         normDiff, tanDiff, bitanDiff;
};


struct MorphTarget
{
	char                      name[256];
	std::vector< MorphDiff >  diffs;
};


class Converter
{
private:

	std::vector< Vertex >        _vertices;
	std::vector< unsigned int >  _indices;
	std::vector< Mesh * >        _meshes;
	std::vector< Joint * >       _joints;
	std::vector< MorphTarget >   _morphTargets;

	float                        _lodDist1, _lodDist2, _lodDist3, _lodDist4;
	unsigned int                 _frameCount;
	unsigned int                 _maxLodLevel;


	Matrix4f getNodeTransform( ColladaDocument &doc, DaeNode &node, unsigned int frame );
	SceneNode *processNode( ColladaDocument &doc, DaeNode &node, SceneNode *parentNode,
	                        Matrix4f transAccum, std::vector< Matrix4f > animTransAccum );
	void calcTangentSpaceBasis( std::vector< Vertex > &vertices );
	void processJoints();
	void processMeshes( ColladaDocument &doc, bool optimize );
	bool writeGeometry( const std::string &name );
	void writeSGNode( const std::string &modelName, SceneNode *node, unsigned int depth, std::ofstream &outf );
	bool writeSceneGraph( const std::string &name );
	void writeAnimFrames( SceneNode &node, FILE *f );

public:

	Converter( float *lodDists );
	~Converter();
	
	bool convertModel( ColladaDocument &doc, bool optimize );
	bool saveModel( const std::string &name );
	
	bool writeMaterials( ColladaDocument &doc, const std::string &name );
	bool hasAnimation();
	bool writeAnimation( const std::string &name );
};

#endif // _converter_H_
