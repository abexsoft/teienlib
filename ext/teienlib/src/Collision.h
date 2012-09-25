#ifndef COLLISION_H
#define COLLISION_H

#include <map>
#include <deque>
#include <utility>
#include <string>

class Entity;

/**
 * Collision library:
 *
 * This code is based on an excelent book, called Real-Time Collision Detection.
 */


#include "Shape.h"

/// ��ư��Ʊ�Τξ���Ƚ�ꡣ�ߤޤäƤƤ⤤����
/**
 * �����˽񤯤٤������ȤǤϤʤ������񤯤Ȥ�����ޤ�ޤǤ����˵�����
 *
 * ���Υ��饹�η�ޤ��:
 *	t�Ͼ�� 0 <= t <= 1
 *	���äơ�������ֶ�� dt �Ǥθ򺹤������
 *	V��Ĵ�����롣V = Vorg * dt
 *
 *	Moving intersect��vel�ϡ�
 *	shapeB��®�٤�0�ˤ��롣�Ĥޤ�
 *	Vel = A.vel - B.vel
 *	V��Ĵ����
 */

namespace scl{

class Collision
{
public:
	Collision();
	virtual ~Collision();
	
	void setDebug(bool aBool){ debug = aBool; }
	
	/// Moving intersetct wrapper
	/**
	 * 1. ®�٤κ�ʬ��ȤꡢshapeB��ߤᡢshapeA������®�٤Τߤˤ��롣
	 * 2. ®�٤����Ĥ�Ʊ̾�ؿ���Ƥ�
	 */
	bool intersectMovingShapes(Shape* shapeA, Vector3D<float> velA,
							   Shape* shapeB, Vector3D<float> velB,
							   float delta, float *t);
	/// Moving intersect
	/**
	 * 1. ������Ƚ�̤��롣
	 * 2. �����ϰϤ�1�ˤ��뤿��ˡ�®�٤��ѹ�����(delta�ܤ���)��
	 * 3. �����˹�碌��ɾ���ؿ���Ƥ֡�
	 */
	bool intersectMovingShapes(Shape* shapeA, Vector3D<float> vel,
							   Shape* shapeB, 
							   float delta, float *t);
	
	//
	// Basic collision function
	//
	
	// Test
	bool testAABBAABB(AABB boxA, AABB boxB);
	
	// Closest Point
	void  closestPtSegment(Segment sg, Vector3D<float> aPos, Vector3D<float> *aXPos);
	float closestPtSegmentSegment(Segment sg1, Segment sg2,
								  float *s, float *t, 
								  Vector3D<float> *c1, Vector3D<float> *c2);
	// Intersect
	/** 
	 * Ray intersetct
	 * ray = p + dir * t
	 * colP = dir * t
	 */
	bool intersectRayPlane(Vector3D<float> p, Vector3D<float> dir, Plane pl,
						   float *t, Vector3D<float> *colP);
	bool intersectRayAABB(Vector3D<float> p, Vector3D<float> dir, AABB box,
						  float *dist, Vector3D<float> *colP);
	bool intersectRaySphere(Vector3D<float> p, Vector3D<float> dir, Sphere sp,
							float *t, Vector3D<float> *colP);
	
	/**
	 * 1. 0 <= *t <= 1
	 * 2. ���ͤޤǤε�Υ�� seg.len() * (*t)
	 */
	bool intersectSegmentSphere(Segment seg, Sphere sp, float *t);
	bool intersectSegmentCapsule(Segment sg, Capsule cap, float *t);
	
private:
	typedef bool (Collision::*ColFuncPtr)(Shape*, Vector3D<float>, Shape*, float *);
	typedef std::map<pair<std::string, std::string>, ColFuncPtr> ColMap;
	
	pair<std::string, std::string> makeStringPair(const char *s1, const char *s2);
	
	ColMap* initCollisionMap();
	ColFuncPtr lookup(const std::string & classA, const std::string & classB);
	
	// Shape���������顢�������ɲ�
	
	bool intersectMovingSphereSphere(Shape* sphereA, Vector3D<float> vel, Shape* sphereB, float *t);
	bool intersectMovingSpherePlane(Shape* sphere, Vector3D<float> vel, Shape* plane, float *t);
	bool intersectMovingSphereAABB(Shape* sphere, Vector3D<float> vel, Shape* aabb, float *t);
	bool intersectMovingAABBAABB(Shape* aabbA, Vector3D<float> vel, Shape* aabbB, float *t);
	bool intersectMovingAABBPlane(Shape* aabb, Vector3D<float> vel, Shape* plane, float *t);
	bool intersectMovingPlanePlane(Shape* planeA, Vector3D<float> vel, Shape* planeB, float *t);
	
	bool intersectMovingPlaneSphere(Shape* pl, Vector3D<float> vel, Shape* sp, float *t)
	{
		return intersectMovingSpherePlane(sp, -vel, pl, t);	
	}
	bool intersectMovingAABBSphere(Shape* box, Vector3D<float> vel, Shape* sp, float *t)
	{
		return intersectMovingSphereAABB(sp, -vel, box, t);
	}
	bool intersectMovingPlaneAABB(Shape* pl, Vector3D<float> vel, Shape* box, float *t)
	{
		return intersectMovingAABBPlane(box, -vel, pl, t);
	}
	
	bool debug;
};

}
#endif
