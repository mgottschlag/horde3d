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

#ifndef _egResource_H_
#define _egResource_H_

#include "egPrerequisites.h"
#include <vector>
#include <map>
#include <string>


// =================================================================================================
// Resource
// =================================================================================================

struct ResourceTypes
{
	enum List
	{
		Undefined = 0,
		SceneGraph,
		Geometry,
		Animation,
		Material,
		Code,
		Shader,
		Texture,
		ParticleEffect,
		Pipeline
	};
};

struct ResourceFlags
{
	enum Flags
	{
		NoQuery = 1,
		NoTexCompression = 2,
		NoTexMipmaps = 4
	};
};

// =================================================================================================

class Resource
{
protected:

	int                  _type;
	std::string          _name;
	ResHandle            _handle;
	int                  _flags;

	uint32               _refCount;  // Number of other objects referencing to this resource
	uint32               _userRefCount;  // Number of handles created by user

	bool                 _loaded;
	bool                 _noQuery;

public:

	Resource( int type, const std::string &name, int flags );
	virtual ~Resource();
	virtual Resource *clone();  // TODO: Implement this for all resource types

	virtual void initDefault();
	virtual void release();
	virtual bool load( const char *data, int size );
	void unload();

	virtual int getParami( int param );
	virtual bool setParami( int param, int value );
	virtual float getParamf( int param );
	virtual bool setParamf( int param, float value );
	virtual const char *getParamstr( int param );
	virtual bool setParamstr( int param, const char *value );

	virtual int getParamItemi( int param, const char *item );
	virtual bool setParamItemi( int param, const char *item, int value );
	virtual float getParamItemf( int param, const char *item );
	virtual bool setParamItemf( int param, const char *item, float value );
	virtual const char *getParamItemstr( int param, const char *item );
	virtual bool setParamItemstr( int param, const char *item, const char *value );

	virtual const void *getData( int param );
	virtual bool updateData( int param, const void *data, int size );

	int &getType() { return _type; }
	int getFlags() { return _flags; }
	const std::string &getName() { return _name; }
	ResHandle getHandle() { return _handle; }
	bool isLoaded() { return _loaded; }
	void addRef() { ++_refCount; }
	void subRef() { --_refCount; }

	friend class ResourceManager;
};

// =================================================================================================

template< class T > class SmartResPtr
{
private:

    T  *_ptr;

	void addRef() { if( _ptr != 0x0 ) _ptr->addRef(); }
	void subRef() { if( _ptr != 0x0 ) _ptr->subRef(); }

public:

	SmartResPtr( T *ptr = 0x0 ) : _ptr( ptr ) { addRef(); }
	SmartResPtr( const SmartResPtr &smp ) : _ptr( smp._ptr ) { addRef(); }
	~SmartResPtr() { subRef(); }

	T &operator*() const { return *_ptr; }
    T *operator->() const { return _ptr; }
	operator T*() const { return _ptr; }
    operator const T*() const { return _ptr; }
	operator bool() const { return _ptr != 0x0; }
	T *getPtr() { return _ptr; }

	SmartResPtr &operator=( const SmartResPtr &smp ) { return *this = smp._ptr; }
	SmartResPtr &operator=( T *ptr )
	{
		subRef(); _ptr = ptr; addRef();
		return *this;
	}
};

typedef SmartResPtr< Resource > PResource;


// =================================================================================================
// Resource Manager
// =================================================================================================

typedef void (*ResTypeInitializationFunc)();
typedef void (*ResTypeReleaseFunc)();
typedef Resource *(*ResTypeFactoryFunc)( const std::string &name, int flags );

struct ResourceRegEntry
{
	std::string                typeString;
	ResTypeInitializationFunc  initializationFunc;  // Called when type is registered
	ResTypeReleaseFunc         releaseFunc;  // Called when type is unregistered
	ResTypeFactoryFunc         factoryFunc;  // Farctory to create resourec object
};

// =================================================================================================

class ResourceManager
{
protected:

	std::vector < Resource * >         _resources;
	std::map< int, ResourceRegEntry >  _registry;  // Registry of resource types

	ResHandle addResource( Resource &res );

public:

	ResourceManager();
	~ResourceManager();

	void registerType( int type, const std::string &typeString, ResTypeInitializationFunc inf,
	                   ResTypeReleaseFunc rf, ResTypeFactoryFunc ff );

	Resource *getNextResource( int type, ResHandle start );
	Resource *findResource( int type, const std::string &name );
	ResHandle addResource( int type, const std::string &name, int flags, bool userCall );
	ResHandle addNonExistingResource( Resource &resource, bool userCall );
	ResHandle cloneResource( ResHandle sourceRes, const std::string &name );
	int removeResource( ResHandle handle, bool userCall );
	void clear();
	ResHandle queryUnloadedResource( int index );
	void releaseUnusedResources();

	Resource *resolveResHandle( ResHandle handle )
		{ return (handle != 0 && (unsigned)(handle - 1) < _resources.size()) ? _resources[handle - 1] : 0x0; }

	std::vector < Resource * > &getResources() { return _resources; }
};

#endif // _egResource_H_
