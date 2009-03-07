// *************************************************************************************************
//
// Horde3D Water Extension
// --------------------------------------------------------
// Copyright (C) 2006-2008 Nicolas Schulz, Volker Wiendl, Mathias Gottschlag
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

#include <sys/time.h>


using namespace std;

namespace Horde3DWater
{
	const int SIZE = 64;

	time_t starttime = -1;

	static float getTime()
	{
		if ( starttime == -1 )
			starttime = time( 0 );
		struct timeval tv;
		if ( gettimeofday(&tv, 0) == -1 )
			perror("gettimeofday");
		tv.tv_sec -= starttime;
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
	}

	const char *vsWaterDebugView =
		"uniform mat4 worldMat;\n"
		"varying vec4 color;\n"
		"uniform vec4 waterInterpolation;\n"
		"attribute float waterHeight;\n"
		"attribute float waterHeight2;\n"
		"void main() {\n"
		"	color = gl_Color;\n"
		"	waterInterpolation.x = cos( waterInterpolation.x * 3.1415 / 2.0 ) / 2.0;\n"
		"	gl_Position = gl_ModelViewProjectionMatrix * worldMat * (gl_Vertex\n"
		"		+ vec4( 0.0, waterHeight * waterInterpolation.x\n"
		"		+ waterHeight2 * ( 1.0 - waterInterpolation.x ), 0.0, 0.0 ) );\n"
		"	//gl_Position = gl_ModelViewProjectionMatrix * worldMat * gl_Vertex;\n"
		"}";

	const char *fsWaterDebugView =
		"varying vec4 color;\n"
		"void main() {\n"
		"	gl_FragColor = color;\n"
		"}\n";

	ShaderCombination WaterNode::debugViewShader;

	
	WaterNode::WaterNode( const WaterNodeTpl &waterTpl ) :
		SceneNode( waterTpl ), _materialRes( waterTpl.matRes ),
		_heightArray( 0x0 ), _vertexBuffer( 0 ), _indexBuffer( 0 )
	{
		_renderable = true;
		camerabuilt = false;

		noise.initialize(128, 128, 128, 1);
		lastframetime = getTime();
		_vertexBuffer = -1;
		_indexBuffer = -1;
		createBuffers();

		_localBBox.getMinCoords() = Vec3f( -1000, 0, -1000 );
		_localBBox.getMaxCoords() = Vec3f( 1000, 1, 1000 );

	}


	WaterNode::~WaterNode()
	{
		delete[] _heightArray;
	}


	SceneNodeTpl *WaterNode::parsingFunc( map< string, string > &attribs )
	{
		map< string, string >::iterator itr;
		WaterNodeTpl *waterTpl = new WaterNodeTpl( "", 0x0 );

		itr = attribs.find( "material" );
		if( itr != attribs.end() )
		{
			uint32 res = Modules::resMan().addResource( ResourceTypes::Material, itr->second, 0, false );
			if( res != 0 )
				waterTpl->matRes = (MaterialResource *)Modules::resMan().resolveResHandle( res );
		}
		itr = attribs.find( "meshQuality" );
		if ( itr != attribs.end() )
		{
			waterTpl->meshQuality = (float)atof( itr->second.c_str() );
		}

		return waterTpl;
	}

	SceneNode *WaterNode::factoryFunc( const SceneNodeTpl &nodeTpl )
	{
		if( nodeTpl.type != SNT_WaterNode ) return 0x0;
		
		return new WaterNode( *(WaterNodeTpl *)&nodeTpl );
	}

	
	void WaterNode::renderFunc( const string &shaderContext, const string &theClass, bool debugView,
								  const Frustum *frust1, const Frustum *frust2, RenderingOrder::List order,
								  int occSet )
	{
		CameraNode *curCam = Modules::renderer().getCurCamera();
		if( curCam == 0x0 ) return;

		Modules::renderer().setMaterial( 0x0, "" );

		// Loop through water queue
		for( uint32 i = 0, s = (uint32)Modules::sceneMan().getRenderableQueue().size(); i < s; ++i )
		{
			if( Modules::sceneMan().getRenderableQueue()[i].type != SNT_WaterNode ) continue;
			
			WaterNode *water = (WaterNode *)Modules::sceneMan().getRenderableQueue()[i].node;
			Vec3f localCamPos( curCam->getAbsTrans().x[12], curCam->getAbsTrans().x[13], curCam->getAbsTrans().x[14] );
			localCamPos = water->_absTrans.inverted() * localCamPos;
			
			if( !debugView )
			{
				if( !water->_materialRes->isOfClass( theClass ) ) continue;
				if( !Modules::renderer().setMaterial( water->_materialRes, shaderContext ) ) continue;
			}
			else
			{
				Modules::renderer().setShader( &debugViewShader );
			}
			
			water->renderWater( localCamPos.x, localCamPos.z );
		}
	}


	bool WaterNode::canAttach( SceneNode &parent )
	{
		return true;
	}



	void WaterNode::renderWater( float x, float z )
	{
		CameraNode *curCam = Modules::renderer().getCurCamera();
		if (!camerabuilt)
		{
			frustum.buildViewFrustum(curCam->getViewMat(), curCam->getProjMat());
			//frustum.buildViewFrustum(Matrix4f::TransMat(0, 100, 0) * Matrix4f::RotMat(-45, 0, 0), 90, 1.3, 1, 500);
			viewMat = curCam->getViewMat();
			projMat = curCam->getProjMat();
			//camerabuilt = true;
		}
		// Get intersections of water plane with the frustum
		int edges[] = {0, 1, 1, 2, 2, 3, 3, 0,
			4, 5, 5, 6, 6, 7, 7, 4,
			0, 4, 1, 5, 2, 6, 3, 7};
		Vec3f corners[8];
		for (int i = 0; i < 8; i++)
		{
			corners[i] = _absTrans.inverted() * frustum.getCorner(i);
			//printf("%d: %f/%f/%f\n", i, corners[i].x, corners[i].y, corners[i].z);
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
				pointcount += 2;
			}
			Plane lower( 0, 1, 0, 0 );
			if ( rayPlaneIntersection( lower, a, b - a, points[pointcount] ) )
			{
				points[pointcount + 1] = points[pointcount];
				points[pointcount + 1].y = 1;
				pointcount += 2;
			}
		}
		// Frustum corners
		for ( int i = 0; i < 8; i++ )
		{
			if ( corners[i].y > 0 && corners[i].y < 1 )
			{
				points[pointcount] = corners[i];
				pointcount++;
			}
		}
		if ( pointcount == 0 ) return;
		// Get screen coordinates
		Matrix4f viewproj = projMat * viewMat;
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

		float time = getTime();
		if ( time - lastframetime >= 100 )
		{
			noise.setFrame( noise.getFrame() + 1 );
			lastframetime = time;
		}
		//destroyBuffers();
		createBuffers( true, min.x, min.y, max.x, max.y );
		float dt = ( time - lastframetime ) / 100;


		int attrib_waterHeight = glGetAttribLocation( Modules::renderer().getCurShader()->shaderObject, "waterHeight" );
		int attrib_waterHeight2 = glGetAttribLocation( Modules::renderer().getCurShader()->shaderObject, "waterHeight2" );
		int attrib_waterNormal = glGetAttribLocation( Modules::renderer().getCurShader()->shaderObject, "waterNormal" );
		int attrib_waterNormal2 = glGetAttribLocation( Modules::renderer().getCurShader()->shaderObject, "waterNormal2" );
		
		// Bind VBO
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _indexBuffer );
		glBindBuffer( GL_ARRAY_BUFFER, _vertexBuffer );
		glVertexPointer( 3, GL_FLOAT, sizeof( float ) * 3, (char *)0 );
		glEnableClientState( GL_VERTEX_ARRAY );
		
		if( attrib_waterHeight >= 0 )
		{
			glVertexAttribPointer( attrib_waterHeight, 1, GL_FLOAT, GL_FALSE, sizeof( float ),
		                       (char *)0 + SIZE * SIZE * sizeof( float ) * 3 );
			glEnableVertexAttribArray( attrib_waterHeight );
		}
		if( attrib_waterHeight2 >= 0 )
		{
			glVertexAttribPointer( attrib_waterHeight2, 1, GL_FLOAT, GL_FALSE, sizeof( float ),
								(char *)0 + SIZE * SIZE * sizeof( float ) * 4 );
			glEnableVertexAttribArray( attrib_waterHeight2 );
		}
		if ( attrib_waterNormal >= 0 )
		{
			glVertexAttribPointer( attrib_waterNormal, 3, GL_FLOAT, GL_FALSE, sizeof( float ) * 3,
								(char *)0 + SIZE * SIZE * sizeof( float ) * 5 );
			glEnableVertexAttribArray( attrib_waterNormal );
			glVertexAttribPointer( attrib_waterNormal2, 3, GL_FLOAT, GL_FALSE, sizeof( float ) * 3,
								(char *)0 + SIZE * SIZE * sizeof( float ) * 8 );
			glEnableVertexAttribArray( attrib_waterNormal2 );
		}
		
		// World transformation
		ShaderCombination *curShader = Modules::renderer().getCurShader();
		if( curShader->uni_worldMat >= 0 )
		{
			glUniformMatrix4fv( curShader->uni_worldMat, 1, false, &_absTrans.x[0] );
		}
		if( curShader->uni_worldNormalMat >= 0 )
		{
			Matrix4f normalMat4 = _absTrans.inverted().transposed();
			float normalMat[9] = { normalMat4.x[0], normalMat4.x[1], normalMat4.x[2],
								   normalMat4.x[4], normalMat4.x[5], normalMat4.x[6],
								   normalMat4.x[8], normalMat4.x[9], normalMat4.x[10] };
			glUniformMatrix3fv( curShader->uni_worldNormalMat, 1, false, normalMat );
		}
		
		int uni_waterInterpolation = glGetUniformLocation( Modules::renderer().getCurShader()->shaderObject, "waterInterpolation" );
		if( uni_waterInterpolation >= 0 )
		{
			glUniform4f( uni_waterInterpolation, dt, 0.0, 0.0, 0.0 );
		}

		//printf("Rendering.\n");

		glDrawElements( GL_TRIANGLES, (SIZE - 1) * (SIZE - 1) * 2 * 3, GL_UNSIGNED_SHORT, (char *)0 );
		Modules::renderer().incStat( EngineStats::BatchCount, 1 );
		Modules::renderer().incStat( EngineStats::TriCount, (SIZE - 1) * (SIZE - 1) * 2.0f );

		glDisableClientState( GL_VERTEX_ARRAY );

		/*glBegin(GL_TRIANGLES);
		glVertex3f(corners[0].x, corners[0].y, corners[0].z);
		glVertex3f(corners[1].x, corners[1].y, corners[1].z);
		glVertex3f(corners[2].x, corners[2].y, corners[2].z);
		glVertex3f(corners[0].x, corners[0].y, corners[0].z);
		glVertex3f(corners[2].x, corners[2].y, corners[2].z);
		glVertex3f(corners[1].x, corners[1].y, corners[1].z);

		glVertex3f(corners[0].x, corners[0].y, corners[0].z);
		glVertex3f(corners[2].x, corners[2].y, corners[2].z);
		glVertex3f(corners[3].x, corners[3].y, corners[3].z);
		glVertex3f(corners[0].x, corners[0].y, corners[0].z);
		glVertex3f(corners[3].x, corners[3].y, corners[3].z);
		glVertex3f(corners[2].x, corners[2].y, corners[2].z);

		glVertex3f(corners[4].x, corners[4].y, corners[4].z);
		glVertex3f(corners[5].x, corners[5].y, corners[5].z);
		glVertex3f(corners[6].x, corners[6].y, corners[6].z);
		glVertex3f(corners[4].x, corners[4].y, corners[4].z);
		glVertex3f(corners[6].x, corners[6].y, corners[6].z);
		glVertex3f(corners[5].x, corners[5].y, corners[5].z);

		glVertex3f(corners[4].x, corners[4].y, corners[4].z);
		glVertex3f(corners[6].x, corners[6].y, corners[6].z);
		glVertex3f(corners[7].x, corners[7].y, corners[7].z);
		glVertex3f(corners[4].x, corners[4].y, corners[4].z);
		glVertex3f(corners[7].x, corners[7].y, corners[7].z);
		glVertex3f(corners[6].x, corners[6].y, corners[6].z);

		glVertex3f(corners[0].x, corners[0].y, corners[0].z);
		glVertex3f(corners[1].x, corners[1].y, corners[1].z);
		glVertex3f(corners[4].x, corners[4].y, corners[4].z);
		glVertex3f(corners[0].x, corners[0].y, corners[0].z);
		glVertex3f(corners[4].x, corners[4].y, corners[4].z);
		glVertex3f(corners[1].x, corners[1].y, corners[1].z);

		glVertex3f(corners[1].x, corners[1].y, corners[1].z);
		glVertex3f(corners[5].x, corners[5].y, corners[5].z);
		glVertex3f(corners[4].x, corners[4].y, corners[4].z);
		glVertex3f(corners[1].x, corners[1].y, corners[1].z);
		glVertex3f(corners[4].x, corners[4].y, corners[4].z);
		glVertex3f(corners[5].x, corners[5].y, corners[5].z);
		glEnd();*/
	}

	void WaterNode::createBuffers( int sizeX, int sizeZ, uint32 *vertexBuffer, uint32 *indexBuffer,
		bool fill, float x1, float y1, float x2, float y2 )
	{
		// Create vertex buffer
		float *vertices = new float[sizeX * sizeZ * 11];
		if ( fill )
		{
			float width = x2 - x1;
			float height = y2 - y1;
			Matrix4f invviewproj = (projMat * viewMat).inverted();
			Vec3f positions[sizeX * sizeZ];
			for (int z = 0; z < sizeZ; z++)
			{
				for (int x = 0; x < sizeX; x++)
				{
					int vertexIndex = z * sizeX + x;
					// Get vertex position
					Vec4f p0(x2 - (width * x / (sizeX - 1)), y1 + (height * z / (sizeZ - 1)), -1, 1);
					Vec4f p1(x2 - (width * x / (sizeX - 1)), y1 + (height * z / (sizeZ - 1)), 1, 1);
					p0 = invviewproj * p0;
					p1 = invviewproj * p1;
					p0.x /= p0.w; p0.y /= p0.w; p0.z /= p0.w;
					p1.x /= p1.w; p1.y /= p1.w; p1.z /= p1.w;
					Vec3f p03( p0.x, p0.y, p0.z );
					Vec3f p13( p1.x, p1.y, p1.z );
					Plane plane( 0, 1, 0, 0 );
					if ( !rayPlaneIntersection( plane, p03, p13 - p03, positions[vertexIndex] ) )
					{
						positions[vertexIndex] = p13;
					}
					// Create vertex
					vertices[vertexIndex * 3] = positions[vertexIndex].x;
					vertices[vertexIndex * 3 + 1] = positions[vertexIndex].y;
					vertices[vertexIndex * 3 + 2] = positions[vertexIndex].z;

					vertices[sizeX * sizeZ * 3 + vertexIndex] = noise.getValue( positions[vertexIndex].x,
						positions[vertexIndex].z, vertices + sizeX * sizeZ * 5 + vertexIndex * 3 );
				}
			}
			int frame = noise.getFrame();
			noise.setFrame( frame + 1 );
			for (int z = 0; z < sizeZ; z++)
			{
				for (int x = 0; x < sizeX; x++)
				{
					float vx = ((float)x / (sizeX - 1) - 0.5) * ((float)z / (sizeZ - 1) * 0.8 + 0.2) + 0.5;
					int vertexIndex = z * sizeX + x;
					Vec3f p = positions[vertexIndex];
					vertices[sizeX * sizeZ * 4 + vertexIndex] = noise.getValue( p.x, p.z, vertices + sizeX * sizeZ * 8 + vertexIndex * 3 );
				}
			}
			noise.setFrame( frame );
		}
		// Upload data
		if ( *vertexBuffer == -1 )
		{
			// Create index buffer
			unsigned short *indices = new unsigned short[(sizeX - 1) * (sizeZ - 1) * 2 * 3];
			for (int z = 0; z < sizeZ - 1; z++)
			{
				for (int x = 0; x < sizeX - 1; x++)
				{
					int indexIndex = z * (sizeX - 1) + x;
					indices[indexIndex * 6] = z * sizeX + x;
					indices[indexIndex * 6 + 2] = z * sizeX + x + 1;
					indices[indexIndex * 6 + 1] = (z + 1) * sizeX + x;
					indices[indexIndex * 6 + 4] = z * sizeX + x + 1;
					indices[indexIndex * 6 + 3] = (z + 1) * sizeX + x + 1;
					indices[indexIndex * 6 + 5] = (z + 1) * sizeX + x;
				}
			}
			*vertexBuffer = Modules::renderer().uploadVertices( vertices, sizeX * sizeZ * sizeof( float ) * 11 );
			*indexBuffer = Modules::renderer().uploadIndices( indices, (sizeX - 1) * (sizeZ - 1) * 2 * 3 * sizeof( short ) );
			delete[] indices;
		}
		else
		{
			Modules::renderer().updateVertices( vertices, 0, sizeX * sizeZ * sizeof( float ) * 11, *vertexBuffer );
		}
		delete[] vertices;
	}
	void WaterNode::createBuffers( bool fill, float x1, float y1, float x2, float y2 )
	{
		createBuffers( SIZE, SIZE, &_vertexBuffer, &_indexBuffer, fill, x1, y1, x2, y2 );
	}
	void WaterNode::destroyBuffers( void )
	{
		Modules::renderer().unloadBuffers( _vertexBuffer, _indexBuffer );
	}

	int WaterNode::getParami( int param )
	{
		switch( param )
		{
		case WaterNodeParams::MaterialRes:
			if( _materialRes != 0x0 ) return _materialRes->getHandle();
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
				Modules::log().writeDebugInfo( "Invalid Material resource for Nature node %i", _handle );
				return false;
			}
			_materialRes = (MaterialResource *)res;
			return true;
		default:
			return SceneNode::setParami( param, value );
		}
	}


	float WaterNode::getParamf( int param )
	{
		switch( param )
		{
		case WaterNodeParams::MeshQuality:
			return 1.0f;
		default:
			return SceneNode::getParamf( param );
		}
	}


	bool WaterNode::setParamf( int param, float value )
	{
		switch( param )
		{
		case WaterNodeParams::MeshQuality: 
			return true;
		default:
			return SceneNode::setParamf( param, value );
		}
	}
}
