#ifndef _MESHSTRIDER_H
#define _MESHSTRIDER_H

#include "Ogre.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#define ASSERT(a) 

class MeshStrider : public btStridingMeshInterface
{
public:
	MeshStrider( Ogre::Mesh * m = 0 ) : mMesh(m){}
 
	void set( Ogre::Mesh * m ) { ASSERT(m); mMesh = m; }
	// inherited interface
	virtual int  getNumSubParts() const;
 
	virtual void getLockedVertexIndexBase(unsigned char **vertexbase, 
					      int& numverts,
					      PHY_ScalarType& type, 
					      int& stride,
					      unsigned char **indexbase,
					      int & indexstride,
					      int& numfaces,
					      PHY_ScalarType& indicestype,
					      int subpart=0);
	virtual void getLockedReadOnlyVertexIndexBase(const unsigned char **vertexbase, 
						      int& numverts,
						      PHY_ScalarType& type, 
						      int& stride,
						      const unsigned char **indexbase,
						      int & indexstride,
						      int& numfaces,
						      PHY_ScalarType& indicestype,
						      int subpart=0) const;
 
    virtual void    unLockVertexBase(int subpart);
    virtual void    unLockReadOnlyVertexBase(int subpart) const;
 
    virtual void    preallocateVertices(int numverts);
    virtual void    preallocateIndices(int numindices);

private:
    Ogre::Mesh * mMesh;
};

#endif
