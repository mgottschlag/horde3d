// *************************************************************************************************
//
// Horde3D Water Extension
// --------------------------------------------------------
// Copyright (C) 2009 Mathias Gottschlag
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

#include "water.h"
#include "egModules.h"
#include "egMaterial.h"
#include "utOpenGL.h"

namespace Horde3DWater
{
	const char *vsWaterDebugView =
		"uniform mat4 worldMat;\n"
		"varying vec4 color;\n"
		"void main() {\n"
		"	color = gl_Color;\n"
		"	gl_Position = gl_ModelViewProjectionMatrix * worldMat * gl_Vertex;\n"
		"}";

	const char *fsWaterDebugView =
		"varying vec4 color;\n"
		"void main() {\n"
		"	gl_FragColor = color;\n"
		"}\n";

	ShaderCombination WaterNode::debugViewShader;

	WaterNode::WaterNode( const WaterNodeTpl &waterTpl ) : SceneNode( waterTpl ),
		_matRes( waterTpl.matRes ), _noiseRes( waterTpl.noiseRes ), _vertexBuffer(0), 
		_indexBuffer(0), vertices(NULL), indices(NULL)
	{
		_renderable = true;
		_localBBox.getMinCoords() = Vec3f( -100, 0, -100 );
		_localBBox.getMaxCoords() = Vec3f( 100, 100, 100 );

		vertices = new float[3 * GRID_SIZE * GRID_SIZE];
		indices = new unsigned short[(GRID_SIZE - 1) * (GRID_SIZE - 1) * 6];

		createBuffers();

		_matRes->setSampler("normalmap", _noiseRes->_normalMap );
	}

	void WaterNode::createBuffers()
	{
		for (int z = 0; z < GRID_SIZE - 1; z++)
		{
			for (int x = 0; x < GRID_SIZE - 1; x++)
			{
				int indexIndex = z * (GRID_SIZE - 1) + x;
				indices[indexIndex * 6] = z * GRID_SIZE + x;
				indices[indexIndex * 6 + 2] = z * GRID_SIZE + x + 1;
				indices[indexIndex * 6 + 1] = (z + 1) * GRID_SIZE + x;
				indices[indexIndex * 6 + 4] = z * GRID_SIZE + x + 1;
				indices[indexIndex * 6 + 3] = (z + 1) * GRID_SIZE + x + 1;
				indices[indexIndex * 6 + 5] = (z + 1) * GRID_SIZE + x;
			}
		}

		_vertexBuffer = Modules::renderer().uploadVertices( vertices, 3 * GRID_SIZE * GRID_SIZE * sizeof( float ) );
		_indexBuffer = Modules::renderer().uploadIndices( indices, (GRID_SIZE - 1) * (GRID_SIZE - 1) * 6 * sizeof( short ) );
	}
	void WaterNode::destroyBuffers()
	{
		Modules::renderer().unloadBuffers( _vertexBuffer, _indexBuffer );
	}
	void WaterNode::updateBuffers( float x1, float y1, float x2, float y2 )
	{
		float width = x2 - x1;
		float height = y2 - y1;
		Matrix4f invviewproj = (_projMat * _viewMat).inverted();
		for (int z = 0; z < GRID_SIZE; z++)
		{
			for (int x = 0; x < GRID_SIZE; x++)
			{
				int vertexIndex = z * GRID_SIZE + x;
				// Get vertex position
				Vec4f p0(x2 - (width * x / (GRID_SIZE - 1)), y1 + (height * z / (GRID_SIZE - 1)), -1, 1);
				Vec4f p1(x2 - (width * x / (GRID_SIZE - 1)), y1 + (height * z / (GRID_SIZE - 1)), 1, 1);
				p0 = invviewproj * p0;
				p1 = invviewproj * p1;
				Vec3f p03( p0.x / p0.w, p0.y / p0.w, p0.z / p0.w );
				Vec3f p13( p1.x / p1.w, p1.y / p1.w, p1.z / p1.w );
				Plane plane( 0, 1, 0, 0 );
				Vec3f position;
				if ( !rayPlaneIntersection( plane, p03, p13 - p03, position ) )
				{
					position = p13;
				}
				position = _absTransInv * position;
				// Create vertex
				vertices[vertexIndex * 3] = position.x;
				vertices[vertexIndex * 3 + 2] = position.z;
				vertices[vertexIndex * 3 + 1] = _noiseRes->getHeight( position.x,
					position.z );
			}
		}
		// Upload data
		Modules::renderer().updateVertices( vertices, 0, GRID_SIZE * GRID_SIZE * sizeof( float ) * 3, _vertexBuffer );
	}
	void WaterNode::render()
	{
		Timer *timer = Modules::stats().getTimer( EngineStats::CustomTime );
		timer->reset();
		timer->setEnabled( true );
		CameraNode *curCam = Modules::renderer().getCurCamera();
		_viewMat = curCam->getViewMat();
		_projMat = curCam->getProjMat();
		_frustum.buildViewFrustum(_viewMat, _projMat);
		// Get intersections of water plane with the frustum
		int edges[] = {0, 1, 1, 2, 2, 3, 3, 0,
			4, 5, 5, 6, 6, 7, 7, 4,
			0, 4, 1, 5, 2, 6, 3, 7};
		Vec3f corners[8];
		for (int i = 0; i < 8; i++)
		{
			corners[i] = _absTransInv * _frustum.getCorner(i);
		}
		Vec3f points[60];
		int pointcount = 0;
		// Frustum edges
		for ( int i = 0; i < 12; i++ )
		{
			const Vec3f &a = corners[edges[i * 2]];
			const Vec3f &b = corners[edges[i * 2 + 1]];
			
			Plane upper( 0, 1, 0, 1 );
			if ( rayPlaneIntersection( upper, a, b - a, points[pointcount] ) )
			{
				points[pointcount + 1] = points[pointcount];
				points[pointcount + 1].y = 0;
				points[pointcount + 1] = _absTrans * points[pointcount + 1];
				points[pointcount] = _absTrans * points[pointcount];
				pointcount += 2;
			}
			Plane lower( 0, 1, 0, 0 );
			if ( rayPlaneIntersection( lower, a, b - a, points[pointcount] ) )
			{
				points[pointcount + 1] = points[pointcount];
				points[pointcount + 1].y = 1;
				points[pointcount + 1] = _absTrans * points[pointcount + 1];
				points[pointcount] = _absTrans * points[pointcount];
				pointcount += 2;
			}
		}
		// Frustum corners
		for ( int i = 0; i < 8; i++ )
		{
			if ( corners[i].y > 0 && corners[i].y < 1 )
			{
				points[pointcount] = _absTrans * corners[i];
				pointcount++;
			}
		}
		if ( pointcount == 0 ) return;
		// Get screen coordinates
		Matrix4f viewproj = _projMat * _viewMat;
		Vec3f min(1, 1, 1);
		Vec3f max(-1, -1, -1);
		for ( int i = 0; i < pointcount; i++ )
		{
			Vec4f point(points[i].x, points[i].y, points[i].z, 1);
			point = viewproj * point;
			points[i].x = point.x / point.w; points[i].y = point.y / point.w; points[i].z = point.z / point.w;
			if ( points[i].x < min.x ) min.x = points[i].x;
			if ( points[i].y < min.y ) min.y = points[i].y;
			if ( points[i].x > max.x ) max.x = points[i].x;
			if ( points[i].y > max.y ) max.y = points[i].y;
		}
		// Get projected grid borders
		updateBuffers( min.x, min.y, max.x, max.y );
		// Bind VBO
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _indexBuffer );
		glBindBuffer( GL_ARRAY_BUFFER, _vertexBuffer );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( float ) * 3, (char *)0 );
		glEnableVertexAttribArray( 0 );

		// World transformation
		ShaderCombination *curShader = Modules::renderer().getCurShader();
		if( curShader->uni_worldMat >= 0 )
		{
			glUniformMatrix4fv( curShader->uni_worldMat, 1, false, &_absTrans.x[0] );
		}
		if( curShader->uni_worldNormalMat >= 0 )
		{
			Matrix4f normalMat4 = _absTransInv.transposed();
			float normalMat[9] = { normalMat4.x[0], normalMat4.x[1], normalMat4.x[2],
								   normalMat4.x[4], normalMat4.x[5], normalMat4.x[6],
								   normalMat4.x[8], normalMat4.x[9], normalMat4.x[10] };
			glUniformMatrix3fv( curShader->uni_worldNormalMat, 1, false, normalMat );
		}


		glDrawElements( GL_TRIANGLES, (GRID_SIZE - 1) * (GRID_SIZE - 1) * 2 * 3, GL_UNSIGNED_SHORT, (char *)0 );
		Modules::stats().incStat( EngineStats::BatchCount, 1 );
		Modules::stats().incStat( EngineStats::TriCount, (GRID_SIZE - 1) * (GRID_SIZE - 1) * 2.0f );

		glDisableVertexAttribArray( 0 );
		timer->setEnabled( false );
	}

	WaterNode::~WaterNode()
	{
		destroyBuffers();
		delete[] indices;
		delete[] vertices;
	}

	SceneNodeTpl *WaterNode::parsingFunc( std::map< std::string, std::string > &attribs )
	{
		return 0;
	}
	SceneNode *WaterNode::factoryFunc( const SceneNodeTpl &nodeTpl )
	{
		if( nodeTpl.type != SNT_WaterNode ) return 0x0;

		return new WaterNode( *(WaterNodeTpl *)&nodeTpl );
	}
	void WaterNode::renderFunc( const std::string &shaderContext, const std::string &theClass, bool debugView,
		const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order, int occSet )
	{
		CameraNode *curCam = Modules::renderer().getCurCamera();
		if( curCam == 0x0 ) return;

		Modules::renderer().setMaterial( 0x0, "" );

		// Loop through water queue
		for( uint32 i = 0, s = (uint32)Modules::sceneMan().getRenderableQueue().size(); i < s; ++i )
		{
			if( Modules::sceneMan().getRenderableQueue()[i].type != SNT_WaterNode ) continue;

			WaterNode *water = (WaterNode *)Modules::sceneMan().getRenderableQueue()[i].node;
			if( !debugView )
			{
				if( !water->_matRes->isOfClass( theClass ) ) continue;
				if( !Modules::renderer().setMaterial( water->_matRes, shaderContext ) ) continue;
			}
			else
			{
				Modules::renderer().setShader( &debugViewShader );
			}

			// World transformation
			water->_absTransInv = water->_absTrans.inverted();
			ShaderCombination *curShader = Modules::renderer().getCurShader();
			if( curShader->uni_worldMat >= 0 )
			{
				glUniformMatrix4fv( curShader->uni_worldMat, 1, false, &water->_absTrans.x[0] );
			}
			if( curShader->uni_worldNormalMat >= 0 )
			{
				Matrix4f normalMat4 = water->_absTransInv.transposed();
				float normalMat[9] = { normalMat4.x[0], normalMat4.x[1], normalMat4.x[2],
									normalMat4.x[4], normalMat4.x[5], normalMat4.x[6],
									normalMat4.x[8], normalMat4.x[9], normalMat4.x[10] };
				glUniformMatrix3fv( curShader->uni_worldNormalMat, 1, false, normalMat );
			}

			water->render();
		}
	}

	bool WaterNode::canAttach( SceneNode &parent )
	{
		return true;
	}
	int WaterNode::getParami( int param )
	{
		switch( param )
		{
		case WaterNodeParams::MaterialRes:
			if( _matRes != 0x0 ) return _matRes->getHandle();
			else return 0;
		default:
			return SceneNode::getParami( param );
		}
	}
	bool WaterNode::setParami( int param, int value )
	{
		Resource *res;
		switch( param )
		{
		case WaterNodeParams::MaterialRes:
			res = Modules::resMan().resolveResHandle( value );
			if( res == 0x0 || res->getType() != ResourceTypes::Material )
			{
				Modules::log().writeDebugInfo( "Invalid Material resource for Water node %i", _handle );
				return false;
			}
			_matRes = (MaterialResource *)res;
			// TODO: Set noise texture
			return true;
		default:
			return SceneNode::setParami( param, value );
		}
	}
}
