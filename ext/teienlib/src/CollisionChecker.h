#ifndef __CollisionChecker_h__
#define __CollisionChecker_h__

#include "Collision.h"
#include <btBulletCollisionCommon.h>

class CollisionChecker
{
public:
	bool intersectMovingShapes(btCollisionShape* shapeA, 
				   btVector3 posA,
				   btVector3 velA,
				   btCollisionShape* shapeB, 
				   btVector3 posB,
				   btVector3 velB,
				   float delta);	


	bool intersectRayAABB(btVector3 p, btVector3 dir, 
			      btBoxShape* box, btVector3 pos,
			      btVector3* colP);

	void closestPtSegment(btVector3 segA, btVector3 segB, 
			      btVector3 pos, btVector3* xPt);

	scl::Shape* convertShape(btCollisionShape* btShape, btVector3 btPos);
	sul::Vector3D<float> convertVector(btVector3 btVec);

private:
	scl::Collision col;
};

#endif
