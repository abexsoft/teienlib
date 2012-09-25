#include "Collision.h"

#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <memory>

#define FLT_MAX 10
#define EPSILON 1.0e-8

#define Max(a, b) (a > b) ? a : b  
#define Min(a, b) (a < b) ? a : b  

namespace scl{

Collision::Collision()
{
	debug = false;
}

Collision::~Collision()
{
}

Collision::ColMap*
Collision::initCollisionMap()
{
	ColMap *cm = new ColMap;
	
	//cout << "Collision:: ColMap new" << endl;
	
	(*cm)[makeStringPair("Sphere", "Sphere")] = &Collision::intersectMovingSphereSphere;
	(*cm)[makeStringPair("Sphere", "Plane")]  = &Collision::intersectMovingSpherePlane;
	(*cm)[makeStringPair("Sphere", "AABB")]	  = &Collision::intersectMovingSphereAABB;
	
	(*cm)[makeStringPair("AABB", "AABB")]	  = &Collision::intersectMovingAABBAABB;
	(*cm)[makeStringPair("AABB", "Plane")]	  = &Collision::intersectMovingAABBPlane;
	(*cm)[makeStringPair("AABB", "Sphere")]	  = &Collision::intersectMovingAABBSphere;
	
	(*cm)[makeStringPair("Plane", "Plane")]	  = &Collision::intersectMovingPlanePlane;
	(*cm)[makeStringPair("Plane", "AABB")]	  = &Collision::intersectMovingPlaneAABB;
	(*cm)[makeStringPair("Plane", "Sphere")]  = &Collision::intersectMovingPlaneSphere;
	
	return cm;
}

pair<std::string, std::string> 
Collision::makeStringPair(const char *s1, const char *s2)
{ 
	std::string str1(s1);
	std::string str2(s2);
	
	pair<std::string, std::string> retPair(str1, str2);
	
	return retPair; 
}

Collision::ColFuncPtr 
Collision::lookup(const std::string & classA, const std::string & classB)
{
	static auto_ptr<ColMap> colMap(initCollisionMap());
	
	ColMap::iterator mapEntry = colMap->find(make_pair(classA, classB));
	
	if (mapEntry == colMap->end())
		return NULL;
	
	return (*mapEntry).second;
}


bool 
Collision::intersectMovingShapes(Shape* shapeA, Vector3D<float> velA,
								 Shape* shapeB, Vector3D<float> velB,
								 float delta, float *t)
{
	return intersectMovingShapes(shapeA, velA - velB, shapeB, delta, t);
}

bool 
Collision::intersectMovingShapes(Shape* shapeA, Vector3D<float> vel,
								 Shape* shapeB, 
								 float delta, float *t)
{
	bool judge = false;
	
	ColFuncPtr cfp = lookup(std::string(shapeA->getClassName()), std::string(shapeB->getClassName()));
	
	if (cfp){
		Vector3D<float> moveVector;
		float aT = 0;
		
		moveVector = vel * delta; ///< moveVector�ϡ�delta���֤�1�Ȥ����Ȥ���®��
		
		try {
			///< aT�ˤϡ�moveVectorʬ�ΰ�ư���֤�1�Ȥ������ξ��ͻ��֤��֤�
			judge = (this->*cfp)(shapeA, moveVector, shapeB, &aT); 
		}
		catch (...) {
			cerr << "Error: exception in intersectMovingShapes()" << endl;
		}
		
		*t = aT * delta; ///< �̾���֤ˤ�ɤ�
	}
	else {
		cerr << "Error: This Collision class does not support this pair of Shapes in intersetcMovingShapes" << endl;
		cerr << "ShapeA: " << shapeA->getClassName() << ", ShapeB: " << shapeB->getClassName() << endl;
	}
	
	if (debug) {
		cout << "judge: ";

		if (judge)
			cout << "true" << endl;
		else
			cout << "false" << endl;
	}
	
	return judge;
}

bool
Collision::intersectMovingSpherePlane(Shape* sphere, Vector3D<float> vel, Shape* plane, float *t)
{
	Sphere sp = *(dynamic_cast<Sphere*>(sphere));
	Plane  pl = *(dynamic_cast<Plane*>(plane));
	
	float dist = (pl.n).dot(sp.c) - pl.d;

	if (fabsf(dist) <= sp.r) {
		*t = 0.0f;
		return true;
	}
	else {
		float denom = (pl.n).dot(vel);

		if (denom * dist >= 0.0f) {
			return false;
		}
		else {
			float r = dist > 0.0f ? sp.r : -sp.r;
			*t = (r - dist) / denom;
			return true;

			/*
			if (*t <= 1)
				return true;
			else
				return false;
			*/
		}
	}
}

bool 
Collision::intersectMovingSphereSphere(Shape* sphereA, Vector3D<float> vel, Shape* sphereB, float *t)
{
	Sphere spA = *(dynamic_cast<Sphere*>(sphereA));
	Sphere spB = *(dynamic_cast<Sphere*>(sphereB));
	
	Sphere spBex = spB;
	
	// �Żߤ����Ƥ���spB��Ⱦ�¤�spA��Ⱦ�¤��ɲä���spA�����ˤ��롣
	spBex.r += spA.r;
	
	Vector3D<float> vN = vel;
	vN.normalize();
	
	Vector3D<float> colP;
	
	// ��spA����vN�����ˤǤ������Ⱦ�³�����spBex�Ȥξ���Ƚ��
	if (intersectRaySphere(spA.c, vN, spBex, t, &colP)) {
		*t = (*t) / vel.len();

		if (*t <= 1)
			return true;
	}
	return false;
}

// AABB��ĺ���򥤥�ǥå���n�ˤ���֤�����ؿ�
Vector3D<float> Corner(AABB b, int n) 
{
	Vector3D<float> p;
	p.x = ((n & 1) ? b.max.x : b.min.x);
	p.y = ((n & 2) ? b.max.y : b.min.y);
	p.z = ((n & 4) ? b.max.z : b.min.z);

	return p;
}

/**
 * AABB���Ф���ư���Ƥ����θ�
 *
 * vel = Sphere.vel - AABB.vel 
 */

bool 
Collision::intersectMovingSphereAABB(Shape* sphere, Vector3D<float> vel, Shape* aabb, float *t)
{
	Sphere sp  = *(dynamic_cast<Sphere*>(sphere));
	AABB   box = *(dynamic_cast<AABB*>(aabb));
	
	// ���Ⱦ��r�ޤ�b���ĥ��������̤Ȥ���������AABB��׻�
	AABB e = box;
	e.min.x -= sp.r; e.min.y -= sp.r; e.min.z -= sp.r;
	e.max.x += sp.r; e.max.y += sp.r; e.max.z += sp.r;
	
	// �����򡢳�ĥ������AABB e���Ф��Ƹ򺹡��������ɤ��e�򳰤���硢�򺹤ʤ��ǽ�λ
	// �����Ǥʤ���С�����p����ӻ���t���̤Ȥ�������
	Vector3D<float> p;
	if (!intersectRayAABB(sp.c, vel, e, t, &p) || *t > 1.0f)
		return false;
	
	// ����p�����γ�¦�˰��֤��Ƥ���b��min�����max���̤�׻�
	// u�����v�Υ��åȤ���Ƥ���ӥåȤν����Ʊ���ˤϤʤ�ʤ����ȡ������
	// �����δ֤Ǿ��ʤ��Ȥ��ĤΥӥåȤ����åȤ���ʤ���Фʤ�ʤ������
	int u = 0, v = 0;
	if (p.x < box.min.x) u |= 1;
	if (p.x > box.max.x) v |= 1;
	if (p.y < box.min.y) u |= 2;
	if (p.y > box.max.y) v |= 2;
	if (p.z < box.min.z) u |= 4;
	if (p.z > box.max.z) v |= 4;
	
	// ��Or��- ���٤ƤΥ��åȤ���Ƥ���ӥåȤ�ӥåȤΥޥ����˰��������(��� �����Ǥ� u + v == u | v)
	int m = u + v;
	
	// ��α�ư�ˤ�äƵ��Ҥ����ľ������ʬ [sp.c, sp.c+vel] ���������
	Vector3D<float> lastPt = sp.c + vel;
	Segment seg(sp.c, lastPt);
	Capsule cap;
	
	// ���٤Ƥ�3�ĤΥӥåȤ����å�(m == 7)����Ƥ����硢p��ĺ���ΰ�ˤ���
	if (m == 7) {
		// �������ʬ [sp.c, sp.c+vel] ��ĺ���ǽв�äƤ���3�Ĥ��դΥ��ץ�����Ф��Ƹ򺹤��Ƥ���
		// 1�İʾ�θ򺹤�����к��ɤλ��֤��֤�
		float tmin = FLT_MAX;
		
		cap.seg.a = Corner(box, v);
		cap.seg.b = Corner(box, v ^ 1);
		cap.r = sp.r;
		if (intersectSegmentCapsule(seg, cap, t))
			tmin = Min(*t, tmin);
		
		cap.seg.a = Corner(box, v);
		cap.seg.b = Corner(box, v^2);
		cap.r = sp.r;
		if (intersectSegmentCapsule(seg, cap, t))
			tmin = Min(*t, tmin);
		
		cap.seg.a = Corner(box, v);
		cap.seg.b = Corner(box, v ^ 4);
		cap.r = sp.r;
		if (intersectSegmentCapsule(seg, cap, t))
			tmin = Min(*t, tmin);
		
		if (tmin == FLT_MAX) 
			return false; // �򺹤ʤ�
		
		*t = tmin;
		return true; // ����t == tmin�ˤ����Ƹ�
	}
	// m�˥��åȤ���Ƥ���ӥåȤ�1�Ĥ����ξ�硢���ä�p�����ΰ����ˤ���
	if ((m & (m - 1)) == 0) {
		// ���⤷�ʤ�����ĥ���줿Ȣ�Ȥθ򺹤������t��
		// �������򺹻��֤Ǥ���
		return true;
	}
	// p�����ΰ�ˤ��롣�դˤ����ƥ��ץ���ȸ򺹤��Ƥ���
	
	cap.seg.a = Corner(box, u^7);
	cap.seg.b = Corner(box, v);
	cap.r = sp.r;
	return intersectSegmentCapsule(seg, cap, t);
}


/// �Żߤ��Ƥ���aabbB���Ф���ư���Ƥ���aabbA�θ�
bool 
Collision::intersectMovingAABBAABB(Shape* aabbA, Vector3D<float> vel, Shape* aabbB, float *t)
{
	if (debug)
		cout << "Debug: intersectMovingAABBAABB" << endl;
	
	AABB* aBox;
	assert(aBox = dynamic_cast<AABB*>(aabbA));
	AABB boxA = *aBox;
	assert(aBox = dynamic_cast<AABB*>(aabbB));
	AABB boxB = *aBox;
	
	/*
	 * ��ͤλ���ˤ�ꡢ®��ȿž
	 * aabbB�ǤϤʤ���aabbA��ߤ��
	 */
	Vector3D<float> v = -vel; 
	
	float tfirst, tlast;
	
	// �ǽ�λ�����'a'�����'b'���ŤʤäƤ����硤����˽�λ
	if (testAABBAABB(boxA, boxB)) {
		*t = tfirst = tlast = 0.0f;
		return true;
	}
	
	// �ǽ�λ����ǽŤʤä�̵���ơ�®�٤�0�ʤ齪λ
	if (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f) {
		if (debug)
			cout << "1" << endl;
		return false;
	}
	
	// �ǽ餪��ӺǸ���ܿ����֤�����
	tfirst = 0.0f;
	tlast = 1.0f;
	
	// �Ƽ����Ф��ơ��ǽ餪��ӺǸ���ܿ����֤򡢤⤷����з��ꤹ��
	// X
	if (v.x == 0.0f)
		if (boxA.max.x < boxB.min.x || boxA.min.x > boxB.max.x) {
			if (debug) cout << "2" << endl;
			return false;
		}
	
	if (v.x < 0.0f) {
		if (boxB.max.x < boxA.min.x) {
			if (debug) cout << "3" << endl;
			return false; // �򺹤Ϥʤ�Υ��Ʊ�ư���Ƥ���
		}
		if (boxA.max.x < boxB.min.x) tfirst = Max((boxA.max.x - boxB.min.x) / v.x, tfirst);
		if (boxB.max.x > boxA.min.x) tlast	= Min((boxA.min.x - boxB.max.x) / v.x, tlast);
	}
	if (v.x > 0.0f) {
		if (boxA.max.x < boxB.min.x) {
			if(debug) cout << "4" << endl;
			return false; // �򺹤Ϥʤ�Υ��Ʊ�ư���Ƥ���
		}
		if (boxB.max.x < boxA.min.x) tfirst = Max((boxA.min.x - boxB.max.x) / v.x, tfirst);
		if (boxA.max.x > boxB.min.x) tlast = Min((boxA.max.x - boxB.min.x) / v.x, tlast);
	}
	// �ǽ���ܿ����Ǹ���ܿ��θ��ȯ��������ϡ��򺹤Ϥ������ʤ�
	if (tfirst > tlast){
		if (debug) cout << "4.5" << endl;
		return false;
	}
	
	
	// Y
	if (v.y == 0.0f)
		if (boxA.max.y < boxB.min.y || boxA.min.y > boxB.max.y) {
			if (debug) cout << "5" << endl;
			return false;
		}
	if (v.y < 0.0f) {
		if (boxB.max.y < boxA.min.y){
			if (debug) cout << "6" << endl;
			return false; // �򺹤Ϥʤ�Υ��Ʊ�ư���Ƥ���
		}
		if (boxA.max.y < boxB.min.y) tfirst = Max((boxA.max.y - boxB.min.y) / v.y, tfirst);
		if (boxB.max.y > boxA.min.y) tlast	= Min((boxA.min.y - boxB.max.y) / v.y, tlast);
	}
	if (v.y > 0.0f) {
		if (boxA.max.y < boxB.min.y){ 
			if (debug) cout << "7" << endl;
			return false; // �򺹤Ϥʤ�Υ��Ʊ�ư���Ƥ���
		}
		if (boxB.max.y < boxA.min.y) tfirst = Max((boxA.min.y - boxB.max.y) / v.y, tfirst);
		if (boxA.max.y > boxB.min.y) tlast = Min((boxA.max.y - boxB.min.y) / v.y, tlast);
	}
	// �ǽ���ܿ����Ǹ���ܿ��θ��ȯ��������ϡ��򺹤Ϥ������ʤ�
	if (tfirst > tlast) {
		if (debug) cout << "7.5" << endl;
		return false;
	}
	
	
	// Z
	if (v.z == 0.0f)
		if (boxA.max.z < boxB.min.z || boxA.min.z > boxB.max.z) {
			if(debug) cout << "8" << endl;
			return false;
		}
	if (v.z < 0.0f) {
		if (boxB.max.z < boxA.min.z) {
			if (debug) cout << "9" << endl;
			return false; // �򺹤Ϥʤ�Υ��Ʊ�ư���Ƥ���
		}
		if (boxA.max.z < boxB.min.z) tfirst = Max((boxA.max.z - boxB.min.z) / v.z, tfirst);
		if (boxB.max.z > boxA.min.z) tlast	= Min((boxA.min.z - boxB.max.z) / v.z, tlast);
	}
	if (v.z > 0.0f) {
		if (boxA.max.z < boxB.min.z){
			if (debug) cout << "10" << endl;
			return false; // �򺹤Ϥʤ�Υ��Ʊ�ư���Ƥ���
		}
		if (boxB.max.z < boxA.min.z) tfirst = Max((boxA.min.z - boxB.max.z) / v.z, tfirst);
		if (boxA.max.z > boxB.min.z) tlast = Min((boxA.max.z - boxB.min.z) / v.z, tlast);
	}
	// �ǽ���ܿ����Ǹ���ܿ��θ��ȯ��������ϡ��򺹤Ϥ������ʤ�
	if (tfirst > tlast){
		if (debug) cout << "10.5" << endl;
		return false;
	}
	
	*t = tfirst;
	return true;	
	
}

bool 
Collision::intersectMovingAABBPlane(Shape* aabb, Vector3D<float> vel, Shape* plane, float *t)
{
	AABB  box = *(dynamic_cast<AABB*>(aabb));
	Plane pl  = *(dynamic_cast<Plane*>(plane));
	
	float e0 = (box.max.x - box.min.x) / 2;
	float e1 = (box.max.y - box.min.y) / 2;
	float e2 = (box.max.z - box.min.z) / 2;
	
	float r = fabs(e0 * pl.n.x + e1 * pl.n.y + e2 * pl.n.z);
	Sphere sp(box.getCenter(), r);
	
	return intersectMovingSpherePlane(&sp, vel, &pl, t);
}

/// Plane vs Plane
/**
 * ʿ�԰ʳ����ͤ��Ƥ���
 */
bool 
Collision::intersectMovingPlanePlane(Shape* planeA, Vector3D<float> vel, Shape* planeB, float *t)
{
	// �̤˻Ȥ��Ĥ��ʤ��Τǡ���˾��ͤ��Ƥ��뤳�Ȥ�
	*t = 0;
	return true;
}

/**
 * Helper function
 */

// [min, max]���ϰ���ޤ�n�򥯥���
inline float 
Clamp(float n, float min, float max) 
{
	if (n < min) return min;
	if (n > max) return max;
	return n;
}

inline void 
Swap(float *a, float *b)
{
	float tmp;
	
	tmp = *a;
	*a = *b;
	*b = tmp;
}


bool 
Collision::testAABBAABB(AABB a, AABB b)
{
	if (a.max.x < b.min.x || a.min.x > b.max.x) return false;
	if (a.max.y < b.min.y || a.min.y > b.max.y) return false;
	if (a.max.z < b.min.z || a.min.z > b.max.z) return false;
	
	return true;
}

void  
Collision::closestPtSegment(Segment sg, Vector3D<float> aPos, Vector3D<float> *aXPos)
{
	Vector3D<float> d = sg.b - sg.a;
	
	float t = (aPos - sg.a).dot(d) / d.dot(d);
	
	if (t < 0.0) t = 0.0;
	if (t > 1.0) t = 1.0;
	
	*aXPos = sg.a + d * t;
}

/**
 * S1(s)=a1+s*(b1-a1)�����S2(t)=a2+t*(b2-a2)��
 * �Ƕ�����C1�����C2��׻���S�����t���֤���
 * �ؿ��η�̤�S1(s)��S2(t)�δ֤ε�Υ��ʿ��
 */
float 
Collision::closestPtSegmentSegment(Segment sg1, Segment sg2,
								   float *s, float *t, Vector3D<float> *c1, Vector3D<float> *c2)
{
	Vector3D<float> d1 = sg1.b - sg1.a; // ��ʬS1�������٥��ȥ�
	Vector3D<float> d2 = sg2.b - sg2.a; // ��ʬS2�������٥��ȥ�
	Vector3D<float> r =	 sg1.a - sg2.a;
	float a = d1.dot(d1); // ��ʬS1�ε�Υ��ʿ�����������
	float e = d2.dot(d2); // ��ʬS2�ε�Υ��ʿ�����������
	float f = d2.dot(r);
	
	// �������뤤��ξ������ʬ�����˽��ष�Ƥ��뤫�ɤ��������å�
	if (a <= EPSILON && e <= EPSILON) {
		// ξ������ʬ�����˽���
		*s = *t = 0.0f;
		*c1 = sg1.a;
		*c2 = sg2.a;
		return (*c1 - *c2).dot(*c1 - *c2);
	}
	if (a <= EPSILON) {
		// �ǽ����ʬ�����˽���
		*s = 0.0f;
		*t = f / e; // s = 0 => t = (b*s + f) / e = f / e
		*t = Clamp(*t, 0.0f, 1.0f);
	} 
	else {
		float c = d1.dot(r);

		if (e <= EPSILON) {
			// 2���ܤ���ʬ�����˽���
			*t = 0.0f;
			*s = Clamp(-c / a, 0.0f, 1.0f); // t = 0 => s = (b*t - c) / a = -c / a
		} 
		else {
			// �����������Ū�ʽ���ξ��򳫻�
			float b = d1.dot(d2);
			float denom = a*e-b*b; // �������
			
			// ��ʬ��ʿ�ԤǤʤ���硢L1���L2���Ф���Ƕ�������׻���������
			// ��ʬS1���Ф��ƥ����ס������Ǥʤ�����Ǥ��s(�����Ǥ�0)������
			if (denom != 0.0f) {
				*s = Clamp((b*f - c*e) / denom, 0.0f, 1.0f);
			} 
			else 
				*s = 0.0f;
			
			// L2���S1(s)���Ф���Ƕ�������ʲ����Ѥ��Ʒ׻�
			// t = Dot((P1+D1*s)-P2,D2) / Dot(D2,D2) = (b*s + f) / e
			*t = (b * (*s) + f) / e;
			
			// t��[0,1]����ˤ���н�λ�������Ǥʤ����t�򥯥��ס�s��t�ο������ͤ��Ф��ưʲ����Ѥ��ƺƷ׻�
			// s = Dot((P2+D2*t)-P1,D1) / Dot(D1,D1)= (t*b - c) / a
			// ������s��[0, 1]���Ф��ƥ�����
			if (*t < 0.0f) {
				*t = 0.0f;
				*s = Clamp(-c / a, 0.0f, 1.0f);
			} 
			else if (*t > 1.0f) {
				*t = 1.0f;
				*s = Clamp((b - c) / a, 0.0f, 1.0f);
			}
		}
	}
	
	*c1 = sg1.a + d1 * (*s);
	*c2 = sg2.a + d2 * (*t);

	return (*c1 - *c2).dot(*c1 - *c2);
}

// ����R(t) = p + t * dir��Plane a���Ф���ɬ���򺹤��Ƥ��롣
// �򺹻���t����Ӹ򺹤��Ƥ�����*colP���֤�
bool
Collision::intersectRayPlane(Vector3D<float> p, Vector3D<float> dir, Plane pl,
							 float *t, Vector3D<float> *colP)
{
	*t = (pl.d - (pl.n).dot(p)) / (pl.n).dot(dir);
	*colP = p + dir * (*t);

	return true;
}


// ����R(t) = p + t*d��AABB a���Ф��Ƹ򺹤��Ƥ��뤫�ɤ������򺹤��Ƥ������
// �򺹻���t����Ӹ򺹤��Ƥ�����*colP���֤�
bool 
Collision::intersectRayAABB(Vector3D<float> p, Vector3D<float> dir, AABB box,
							float *t, Vector3D<float> *colP)
{
	*t = 0.0f;			 // -FLT_MAX�����ꤷ��ľ���ˤ�����ǽ�θ򺹤�����
	float tmax = 10000.0f;	// (��ʬ���Ф���)��������ư���뤳�ȤΤǤ������ε�Υ������
	
	// 3�ĤΤ��٤ƥ���֤��Ф���
	// x
	if (fabsf(dir.x) < EPSILON) {
		// �����ϥ���֤��Ф���ʿ�ԡ�����������֤���ˤʤ���и򺹤ʤ�
		if (p.x < box.min.x || p.x > box.max.x) 
			return false;
	} 
	else {
		// ����֤ζᤤʿ�̤���ӱ�ʿ�̤ȸ򺹤��������t���ͤ�׻�
		float ood = 1.0f / dir.x;
		float t1 = (box.min.x - p.x) * ood;
		float t2 = (box.max.x - p.x) * ood;
		// t1���ᤤʿ�̤Ȥθ򺹡�t2����ʿ�̤Ȥθ򺹤Ȥʤ�
		if (t1 > t2) Swap(&t1, &t2);
		// ����֤θ򺹤��Ƥ���ֳ֤Ȥθ򺹤�׻�
		if (t1 > *t) *t = t1;
		if (t2 < tmax)	tmax  = t2;
		// ����֤˸򺹤��ʤ����Ȥ�ʬ����о��ͤϤʤ��ΤǤ����˽�λ
		if (*t > tmax) 
			return false;
	}
	
	// y
	if (fabsf(dir.y) < EPSILON) {
		// �����ϥ���֤��Ф���ʿ�ԡ�����������֤���ˤʤ���и򺹤ʤ�
		if (p.y < box.min.y || p.y > box.max.y) 
			return false;
	} 
	else {
		// ����֤ζᤤʿ�̤���ӱ�ʿ�̤ȸ򺹤��������t���ͤ�׻�
		float ood = 1.0f / dir.y;
		float t1 = (box.min.y - p.y) * ood;
		float t2 = (box.max.y - p.y) * ood;
		// t1���ᤤʿ�̤Ȥθ򺹡�t2����ʿ�̤Ȥθ򺹤Ȥʤ�
		if (t1 > t2) Swap(&t1, &t2);
		// ����֤θ򺹤��Ƥ���ֳ֤Ȥθ򺹤�׻�
		if (t1 > *t) *t = t1;
		if (t2 < tmax)	tmax = t2;
		// ����֤˸򺹤��ʤ����Ȥ�ʬ����о��ͤϤʤ��ΤǤ����˽�λ
		if (*t > tmax) 
			return false;
	}
	
	// z
	if (fabsf(dir.z) < EPSILON) {
		// �����ϥ���֤��Ф���ʿ�ԡ�����������֤���ˤʤ���и򺹤ʤ�
		if (p.z < box.min.z || p.z > box.max.z) 
			return false;
	} 
	else {
		// ����֤ζᤤʿ�̤���ӱ�ʿ�̤ȸ򺹤��������t���ͤ�׻�
		float ood = 1.0f / dir.z;
		float t1 = (box.min.z - p.z) * ood;
		float t2 = (box.max.z - p.z) * ood;
		// t1���ᤤʿ�̤Ȥθ򺹡�t2����ʿ�̤Ȥθ򺹤Ȥʤ�
		if (t1 > t2) Swap(&t1, &t2);
		// ����֤θ򺹤��Ƥ���ֳ֤Ȥθ򺹤�׻�
		if (t1 > *t) *t = t1;
		if (t2 < tmax)	tmax = t2;
		// ����֤˸򺹤��ʤ����Ȥ�ʬ����о��ͤϤʤ��ΤǤ����˽�λ
		if (*t > tmax) 
			return false;
	}
	
	// ������3�ĤΥ���֤��٤Ƥ˸򺹤��Ƥ��롣��(q)�ȸ򺹤�t����(tmin)���֤�
	*colP = p + dir * (*t);

	return true;
}

// ����R(t) = p + t*dir��Sphere sp���Ф��Ƹ򺹤��Ƥ��뤫�ɤ������򺹤��Ƥ������
// �򺹤ε�Υt(*dist)����Ӹ򺹤��Ƥ�����*colP���֤�
// dir������������
bool 
Collision::intersectRaySphere(Vector3D<float> p, Vector3D<float> dir, Sphere sp,
							  float *t, Vector3D<float> *colP)
{
	Vector3D<float> m = p - sp.c;
	float b = m.dot(dir);
	float c = m.dot(m) - sp.r * sp.r;
	
	// r�θ�����s�γ�¦�ˤ���(c > 0)��r��s����Υ��Ƥ���������ؤ��Ƥ�����(b > 0)�˽�λ
	if (c > 0.0f && b > 0.0f) 
		return false;
	
	float discr = b * b - c ;
	// ���Ƚ�̼��ϸ�������򳰤�Ƥ��뤳�Ȥ˰���
	if (discr < 0.0f) 
		return false;
	
	// ����Ǹ����ϵ�ȸ򺹤��Ƥ��뤳�Ȥ�ʬ���ꡢ�򺹤���Ǿ�����t��׻�
	*t = -b - sqrtf(discr);
	
	// t����Ǥ����硢�����ϵ����¦���鳫�Ϥ��Ƥ���Τ�t�򥼥�˥�����
	if (*t < 0.0f) 
		*t = 0.0f;
	
	*colP = p + dir * (*t);
	
	return true;
}


/**
 * ��ʬS(t)=sa+t(sb-sa), 0<=t<=1�Ρ�����Ф����
 *
 */
bool
Collision::intersectSegmentSphere(Segment seg, Sphere sp, float *t)
{
	Vector3D<float> d = seg.b - seg.a;
	Vector3D<float> dir = d;
	dir.normalize();
	
	Vector3D<float> colP;
	if (intersectRaySphere(seg.a, dir, sp, t, &colP)) {
		*t = (*t) / d.len();  // dir������������Ƥ���Τǡ�(*t)�ϵ�Υ
		if (*t <= 1)
			return true;   
		else
			return false;
	}
	else
		return false;
}

/**
 * ��ʬS(t)=sa+t(sb-sa), 0<=t<=1�Ρ����ץ�����Ф����
 */
bool 
Collision::intersectSegmentCapsule(Segment seg, Capsule cap, float *t)
{
	Vector3D<float> d = cap.seg.b - cap.seg.a;
	Vector3D<float> m = seg.a - cap.seg.a;
	Vector3D<float> n = seg.b - seg.a;
	float md = m.dot(d);
	float nd = n.dot(d);
	float dd = d.dot(d);
	
	// ��ʬ���Τ��ɤ��餫�α�������̤��Ф��ƴ����˳�¦�ˤ��뤫�ɤ�����Ƚ��
	if (md < 0.0f && md + nd < 0.0f) { 
		// ��ʬ�������'a'��¦�γ�¦�ˤ���
		
		// ���ץ���ʤΤ�
		Sphere sp(cap.seg.a, cap.r);
		if(intersectSegmentSphere(seg, sp, t))
			return true;
		return false; 
	}
	if (md > dd && md + nd > dd) { 
		// ��ʬ�������'b'��¦�γ�¦�ˤ���
		
		// ���ץ���ʤΤ�
		Sphere sp(cap.seg.b, cap.r);
		if(intersectSegmentSphere(seg, sp, t))
			return true;	
		return false;	  
	}
	
	// ��ʬ�Τɤ��餫��ü�⤷����ξ���α������̶����ˤ���
	float nn = n.dot(n);
	float mn = m.dot(n);
	float a = dd * nn - nd * nd;
	float k = m.dot(m) - cap.r * cap.r;
	float c = dd * k - md * md;
	// ��ʬ������μ����Ф���ʿ�Ԥ����äƤ��뤫�ɤ�����Ƚ��
	if (fabsf(a) < EPSILON) {	
		if (c > 0.0f) // 'a'�������ʬ�ϱ���γ�¦�ˤ���
			return false; 
		
		// �������ʬ������ȸ򺹤��Ƥ��뤳�Ȥ�ʬ���ä��Τǡ��ɤΤ褦�˸򺹤��Ƥ��뤫��Ĵ�٤�
		if (md < 0.0f){ 
			// ��ʬ��'a'��¦�����̤ȸ򺹤��Ƥ���
			// ���ץ���ʤΤ�
			Sphere sp(cap.seg.a, cap.r);
			intersectSegmentSphere(seg, sp, t);
		}
		else if (md > dd){
			// ��ʬ��'b'��¦�����̤ȸ򺹤��Ƥ���
			// ���ץ���ʤΤ�
			Sphere sp(cap.seg.b, cap.r);
			intersectSegmentSphere(seg, sp, t);
		}
		else {
			// 'a'�ϱ������¦�ˤ���
			*t = 0.0f; 
		}
		return true;
	}
	
	float b = dd * mn - nd * md;
	float discr = b * b - a * c;
	if (discr < 0.0f) 
		return false; // �¿��򤬤ʤ��ΤǸ򺹤Ϥʤ�
	
	*t = (-b - sqrtf(discr)) / a;
	if (*t < 0.0f || *t > 1.0f) 
		return false; // �򺹤���ʬ�γ�¦�ˤ���
	
	if (md + (*t) * nd < 0.0f) {
		// �����'a'��¦�γ�¦�Ǹ�
		// ���ץ���ʤΤ�
		Sphere sp(cap.seg.a, cap.r);
		return intersectSegmentSphere(seg, sp, t);
	} 
	else if (md + (*t) * nd > dd) {
		// �����'q'��¦�γ�¦�Ǹ�
		// ���ץ���ʤΤ�
		Sphere sp(cap.seg.b, cap.r);
		return intersectSegmentSphere(seg, sp, t);
	}
	// ��ʬ�����̤δ֤δ֤Ǹ򺹤��Ƥ���Τǡ�t��������
	return true;	
}

}
