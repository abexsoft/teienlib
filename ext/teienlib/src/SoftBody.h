#ifndef _UTILS_H
#define _UTILS_H

#include "Ogre.h"
#include "btSoftBody.h"

class SoftBody
{
public:
	static btSoftBody* createFromMesh(Ogre::Mesh* mesh, btSoftBodyWorldInfo* worldInfo);
	static void updateOgreMesh(Ogre::Mesh* mesh, btSoftBody* softBody);
};

#endif
