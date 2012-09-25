#include "CollisionChecker.h"

bool
CollisionChecker::intersectMovingShapes(btCollisionShape* shapeA, 
					btVector3 posA,
					btVector3 velA,
					btCollisionShape* shapeB, 
					btVector3 posB,
					btVector3 velB,
					float delta)
{
	float t;
	return col.intersectMovingShapes(convertShape(shapeA, posA),
					 convertVector(velA),
					 convertShape(shapeB, posB),
					 convertVector(velB),
					 delta, &t);
}


bool 
CollisionChecker::intersectRayAABB(btVector3 p, btVector3 dir, 
				   btBoxShape* box, btVector3 pos,
				   btVector3* colP)
{
	float t;
	sul::Vector3D<float> xPt;
	bool bl =  col.intersectRayAABB(convertVector(p),
					convertVector(dir),
					*(scl::AABB*)convertShape(box, pos),
					&t, &xPt);
	colP->setValue(xPt.x, xPt.y, xPt.z);
	return bl;
}

void
CollisionChecker::closestPtSegment(btVector3 segA, btVector3 segB, 
				   btVector3 pos, btVector3* colP)
{
	sul::Vector3D<float> xPt;
	col.closestPtSegment(scl::Segment(convertVector(segA), convertVector(segB)),
			     convertVector(pos),  &xPt);
	colP->setValue(xPt.x, xPt.y, xPt.z);
}


scl::Shape*
CollisionChecker::convertShape(btCollisionShape* btShape, btVector3 btPos)
{
	if (btBoxShape* btBox = dynamic_cast<btBoxShape*>(btShape)) {
		scl::AABB* shape = new scl::AABB();
		shape->setPos(convertVector(btPos), 
			      convertVector(btBox->getHalfExtentsWithMargin()));
		return shape;
	}
	else {
		std::cerr << __PRETTY_FUNCTION__ << ": This shape is not supported." << std::endl;
		return NULL;
	}
}


sul::Vector3D<float>
CollisionChecker::convertVector(btVector3 btVec)
{
	return sul::Vector3D<float>(btVec.x(), btVec.y(), btVec.z());
}
