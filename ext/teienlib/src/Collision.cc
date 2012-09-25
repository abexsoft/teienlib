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
		
		moveVector = vel * delta; ///< moveVectorは、delta時間を1としたときの速度
		
		try {
			///< aTには、moveVector分の移動時間を1とした時の衝突時間が返る
			judge = (this->*cfp)(shapeA, moveVector, shapeB, &aT); 
		}
		catch (...) {
			cerr << "Error: exception in intersectMovingShapes()" << endl;
		}
		
		*t = aT * delta; ///< 通常時間にもどす
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
	
	// 静止させているspBの半径にspAの半径を追加し、spAを点にする。
	spBex.r += spA.r;
	
	Vector3D<float> vN = vel;
	vN.normalize();
	
	Vector3D<float> colP;
	
	// 点spAからvN方向にでる光線と半径拡大版spBexとの衝突判定
	if (intersectRaySphere(spA.c, vN, spBex, t, &colP)) {
		*t = (*t) / vel.len();

		if (*t <= 1)
			return true;
	}
	return false;
}

// AABBの頂点をインデックスnにより返す補助関数
Vector3D<float> Corner(AABB b, int n) 
{
	Vector3D<float> p;
	p.x = ((n & 1) ? b.max.x : b.min.x);
	p.y = ((n & 2) ? b.max.y : b.min.y);
	p.z = ((n & 4) ? b.max.z : b.min.z);

	return p;
}

/**
 * AABBに対する動いている球の交差
 *
 * vel = Sphere.vel - AABB.vel 
 */

bool 
Collision::intersectMovingSphereAABB(Shape* sphere, Vector3D<float> vel, Shape* aabb, float *t)
{
	Sphere sp  = *(dynamic_cast<Sphere*>(sphere));
	AABB   box = *(dynamic_cast<AABB*>(aabb));
	
	// 球の半径rまでbを拡張させた結果として得られるAABBを計算
	AABB e = box;
	e.min.x -= sp.r; e.min.y -= sp.r; e.min.z -= sp.r;
	e.max.x += sp.r; e.max.y += sp.r; e.max.z += sp.r;
	
	// 光線を、拡張させたAABB eに対して交差。光線がどれもeを外れる場合、交差なしで終了
	// そうでなければ、交差点pおよび時間tを結果として得る
	Vector3D<float> p;
	if (!intersectRayAABB(sp.c, vel, e, t, &p) || *t > 1.0f)
		return false;
	
	// 交差点pがその外側に位置しているbのminおよびmaxの面を計算
	// uおよびvのセットされているビットの集合は同じにはならないこと、および
	// それらの間で少なくとも一つのビットがセットされなければならないに注意
	int u = 0, v = 0;
	if (p.x < box.min.x) u |= 1;
	if (p.x > box.max.x) v |= 1;
	if (p.y < box.min.y) u |= 2;
	if (p.y > box.max.y) v |= 2;
	if (p.z < box.min.z) u |= 4;
	if (p.z > box.max.z) v |= 4;
	
	// 「Or」- すべてのセットされているビットをビットのマスクに一緒に入れる(注意 ここでは u + v == u | v)
	int m = u + v;
	
	// 球の運動によって記述される直線の線分 [sp.c, sp.c+vel] を定義する
	Vector3D<float> lastPt = sp.c + vel;
	Segment seg(sp.c, lastPt);
	Capsule cap;
	
	// すべての3つのビットがセット(m == 7)されている場合、pは頂点領域にある
	if (m == 7) {
		// これで線分 [sp.c, sp.c+vel] が頂点で出会っている3つの辺のカプセルに対して交差している
		// 1つ以上の交差があれば最良の時間を返す
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
			return false; // 交差なし
		
		*t = tmin;
		return true; // 時間t == tminにおいて交差
	}
	// mにセットされているビットが1つだけの場合、従ってpは面領域の中にある
	if ((m & (m - 1)) == 0) {
		// 何もしない。拡張された箱との交差する時間tは
		// 正しい交差時間である
		return true;
	}
	// pは辺領域にある。辺においてカプセルと交差している
	
	cap.seg.a = Corner(box, u^7);
	cap.seg.b = Corner(box, v);
	cap.r = sp.r;
	return intersectSegmentCapsule(seg, cap, t);
}


/// 静止しているaabbBに対する動いているaabbAの交差
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
	 * 大人の事情により、速度反転
	 * aabbBではなく、aabbAを止める
	 */
	Vector3D<float> v = -vel; 
	
	float tfirst, tlast;
	
	// 最初の時点で'a'および'b'が重なっている場合，早期に終了
	if (testAABBAABB(boxA, boxB)) {
		*t = tfirst = tlast = 0.0f;
		return true;
	}
	
	// 最初の時点で重なって無くて、速度が0なら終了
	if (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f) {
		if (debug)
			cout << "1" << endl;
		return false;
	}
	
	// 最初および最後の接触時間を初期化
	tfirst = 0.0f;
	tlast = 1.0f;
	
	// 各軸に対して、最初および最後の接触時間を、もしあれば決定する
	// X
	if (v.x == 0.0f)
		if (boxA.max.x < boxB.min.x || boxA.min.x > boxB.max.x) {
			if (debug) cout << "2" << endl;
			return false;
		}
	
	if (v.x < 0.0f) {
		if (boxB.max.x < boxA.min.x) {
			if (debug) cout << "3" << endl;
			return false; // 交差はなく離れて運動している
		}
		if (boxA.max.x < boxB.min.x) tfirst = Max((boxA.max.x - boxB.min.x) / v.x, tfirst);
		if (boxB.max.x > boxA.min.x) tlast	= Min((boxA.min.x - boxB.max.x) / v.x, tlast);
	}
	if (v.x > 0.0f) {
		if (boxA.max.x < boxB.min.x) {
			if(debug) cout << "4" << endl;
			return false; // 交差はなく離れて運動している
		}
		if (boxB.max.x < boxA.min.x) tfirst = Max((boxA.min.x - boxB.max.x) / v.x, tfirst);
		if (boxA.max.x > boxB.min.x) tlast = Min((boxA.max.x - boxB.min.x) / v.x, tlast);
	}
	// 最初の接触が最後の接触の後に発生する場合は、交差はあり得ない
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
			return false; // 交差はなく離れて運動している
		}
		if (boxA.max.y < boxB.min.y) tfirst = Max((boxA.max.y - boxB.min.y) / v.y, tfirst);
		if (boxB.max.y > boxA.min.y) tlast	= Min((boxA.min.y - boxB.max.y) / v.y, tlast);
	}
	if (v.y > 0.0f) {
		if (boxA.max.y < boxB.min.y){ 
			if (debug) cout << "7" << endl;
			return false; // 交差はなく離れて運動している
		}
		if (boxB.max.y < boxA.min.y) tfirst = Max((boxA.min.y - boxB.max.y) / v.y, tfirst);
		if (boxA.max.y > boxB.min.y) tlast = Min((boxA.max.y - boxB.min.y) / v.y, tlast);
	}
	// 最初の接触が最後の接触の後に発生する場合は、交差はあり得ない
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
			return false; // 交差はなく離れて運動している
		}
		if (boxA.max.z < boxB.min.z) tfirst = Max((boxA.max.z - boxB.min.z) / v.z, tfirst);
		if (boxB.max.z > boxA.min.z) tlast	= Min((boxA.min.z - boxB.max.z) / v.z, tlast);
	}
	if (v.z > 0.0f) {
		if (boxA.max.z < boxB.min.z){
			if (debug) cout << "10" << endl;
			return false; // 交差はなく離れて運動している
		}
		if (boxB.max.z < boxA.min.z) tfirst = Max((boxA.min.z - boxB.max.z) / v.z, tfirst);
		if (boxA.max.z > boxB.min.z) tlast = Min((boxA.max.z - boxB.min.z) / v.z, tlast);
	}
	// 最初の接触が最後の接触の後に発生する場合は、交差はあり得ない
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
 * 平行以外衝突している
 */
bool 
Collision::intersectMovingPlanePlane(Shape* planeA, Vector3D<float> vel, Shape* planeB, float *t)
{
	// 別に使うつもりないので、常に衝突していることに
	*t = 0;
	return true;
}

/**
 * Helper function
 */

// [min, max]の範囲内までnをクランプ
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
 * S1(s)=a1+s*(b1-a1)およびS2(t)=a2+t*(b2-a2)の
 * 最近接点C1およびC2を計算、Sおよびtを返す。
 * 関数の結果はS1(s)とS2(t)の間の距離の平方
 */
float 
Collision::closestPtSegmentSegment(Segment sg1, Segment sg2,
								   float *s, float *t, Vector3D<float> *c1, Vector3D<float> *c2)
{
	Vector3D<float> d1 = sg1.b - sg1.a; // 線分S1の方向ベクトル
	Vector3D<float> d2 = sg2.b - sg2.a; // 線分S2の方向ベクトル
	Vector3D<float> r =	 sg1.a - sg2.a;
	float a = d1.dot(d1); // 線分S1の距離の平方、常に非負
	float e = d2.dot(d2); // 線分S2の距離の平方、常に非負
	float f = d2.dot(r);
	
	// 片方あるいは両方の線分が点に縮退しているかどうかチェック
	if (a <= EPSILON && e <= EPSILON) {
		// 両方の線分が点に縮退
		*s = *t = 0.0f;
		*c1 = sg1.a;
		*c2 = sg2.a;
		return (*c1 - *c2).dot(*c1 - *c2);
	}
	if (a <= EPSILON) {
		// 最初の線分が点に縮退
		*s = 0.0f;
		*t = f / e; // s = 0 => t = (b*s + f) / e = f / e
		*t = Clamp(*t, 0.0f, 1.0f);
	} 
	else {
		float c = d1.dot(r);

		if (e <= EPSILON) {
			// 2番目の線分が点に縮退
			*t = 0.0f;
			*s = Clamp(-c / a, 0.0f, 1.0f); // t = 0 => s = (b*t - c) / a = -c / a
		} 
		else {
			// ここから一般的な縮退の場合を開始
			float b = d1.dot(d2);
			float denom = a*e-b*b; // 常に非負
			
			// 線分が平行でない場合、L1上のL2に対する最近接点を計算、そして
			// 線分S1に対してクランプ。そうでない場合は任意s(ここでは0)を選択
			if (denom != 0.0f) {
				*s = Clamp((b*f - c*e) / denom, 0.0f, 1.0f);
			} 
			else 
				*s = 0.0f;
			
			// L2上のS1(s)に対する最近接点を以下を用いて計算
			// t = Dot((P1+D1*s)-P2,D2) / Dot(D2,D2) = (b*s + f) / e
			*t = (b * (*s) + f) / e;
			
			// tが[0,1]の中にあれば終了。そうでなければtをクランプ、sをtの新しい値に対して以下を用いて再計算
			// s = Dot((P2+D2*t)-P1,D1) / Dot(D1,D1)= (t*b - c) / a
			// そしてsを[0, 1]に対してクランプ
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

// 光線R(t) = p + t * dirはPlane aに対して必ず交差している。
// 交差時のtおよび交差している点*colPを返す
bool
Collision::intersectRayPlane(Vector3D<float> p, Vector3D<float> dir, Plane pl,
							 float *t, Vector3D<float> *colP)
{
	*t = (pl.d - (pl.n).dot(p)) / (pl.n).dot(dir);
	*colP = p + dir * (*t);

	return true;
}


// 光線R(t) = p + t*dがAABB aに対して交差しているかどうか、交差している時、
// 交差時のtおよび交差している点*colPを返す
bool 
Collision::intersectRayAABB(Vector3D<float> p, Vector3D<float> dir, AABB box,
							float *t, Vector3D<float> *colP)
{
	*t = 0.0f;			 // -FLT_MAXに設定して直線における最初の交差を得る
	float tmax = 10000.0f;	// (線分に対して)光線が移動することのできる最大の距離に設定
	
	// 3つのすべてスラブに対して
	// x
	if (fabsf(dir.x) < EPSILON) {
		// 光線はスラブに対して平行。原点がスラブの中になければ交差なし
		if (p.x < box.min.x || p.x > box.max.x) 
			return false;
	} 
	else {
		// スラブの近い平面および遠い平面と交差する光線のtの値を計算
		float ood = 1.0f / dir.x;
		float t1 = (box.min.x - p.x) * ood;
		float t2 = (box.max.x - p.x) * ood;
		// t1が近い平面との交差、t2が遠い平面との交差となる
		if (t1 > t2) Swap(&t1, &t2);
		// スラブの交差している間隔との交差を計算
		if (t1 > *t) *t = t1;
		if (t2 < tmax)	tmax  = t2;
		// スラブに交差がないことが分かれば衝突はないのですぐに終了
		if (*t > tmax) 
			return false;
	}
	
	// y
	if (fabsf(dir.y) < EPSILON) {
		// 光線はスラブに対して平行。原点がスラブの中になければ交差なし
		if (p.y < box.min.y || p.y > box.max.y) 
			return false;
	} 
	else {
		// スラブの近い平面および遠い平面と交差する光線のtの値を計算
		float ood = 1.0f / dir.y;
		float t1 = (box.min.y - p.y) * ood;
		float t2 = (box.max.y - p.y) * ood;
		// t1が近い平面との交差、t2が遠い平面との交差となる
		if (t1 > t2) Swap(&t1, &t2);
		// スラブの交差している間隔との交差を計算
		if (t1 > *t) *t = t1;
		if (t2 < tmax)	tmax = t2;
		// スラブに交差がないことが分かれば衝突はないのですぐに終了
		if (*t > tmax) 
			return false;
	}
	
	// z
	if (fabsf(dir.z) < EPSILON) {
		// 光線はスラブに対して平行。原点がスラブの中になければ交差なし
		if (p.z < box.min.z || p.z > box.max.z) 
			return false;
	} 
	else {
		// スラブの近い平面および遠い平面と交差する光線のtの値を計算
		float ood = 1.0f / dir.z;
		float t1 = (box.min.z - p.z) * ood;
		float t2 = (box.max.z - p.z) * ood;
		// t1が近い平面との交差、t2が遠い平面との交差となる
		if (t1 > t2) Swap(&t1, &t2);
		// スラブの交差している間隔との交差を計算
		if (t1 > *t) *t = t1;
		if (t2 < tmax)	tmax = t2;
		// スラブに交差がないことが分かれば衝突はないのですぐに終了
		if (*t > tmax) 
			return false;
	}
	
	// 光線が3つのスラブすべてに交差している。点(q)と交差のtの値(tmin)を返す
	*colP = p + dir * (*t);

	return true;
}

// 光線R(t) = p + t*dirがSphere spに対して交差しているかどうか、交差している時、
// 交差の距離t(*dist)および交差している点*colPを返す
// dirは正規化前提。
bool 
Collision::intersectRaySphere(Vector3D<float> p, Vector3D<float> dir, Sphere sp,
							  float *t, Vector3D<float> *colP)
{
	Vector3D<float> m = p - sp.c;
	float b = m.dot(dir);
	float c = m.dot(m) - sp.r * sp.r;
	
	// rの原点がsの外側にあり(c > 0)、rがsから離れていく方向を指している場合(b > 0)に終了
	if (c > 0.0f && b > 0.0f) 
		return false;
	
	float discr = b * b - c ;
	// 負の判別式は光線が球を外れていることに一致
	if (discr < 0.0f) 
		return false;
	
	// これで光線は球と交差していることが分かり、交差する最小の値tを計算
	*t = -b - sqrtf(discr);
	
	// tが負である場合、光線は球の内側から開始しているのでtをゼロにクランプ
	if (*t < 0.0f) 
		*t = 0.0f;
	
	*colP = p + dir * (*t);
	
	return true;
}


/**
 * 線分S(t)=sa+t(sb-sa), 0<=t<=1の、球に対する交差
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
		*t = (*t) / d.len();  // dirは正規化されているので、(*t)は距離
		if (*t <= 1)
			return true;   
		else
			return false;
	}
	else
		return false;
}

/**
 * 線分S(t)=sa+t(sb-sa), 0<=t<=1の、カプセルに対する交差
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
	
	// 線分全体がどちらかの円柱の底面に対して完全に外側にあるかどうかを判定
	if (md < 0.0f && md + nd < 0.0f) { 
		// 線分が円柱の'a'の側の外側にある
		
		// カプセルなので
		Sphere sp(cap.seg.a, cap.r);
		if(intersectSegmentSphere(seg, sp, t))
			return true;
		return false; 
	}
	if (md > dd && md + nd > dd) { 
		// 線分が円柱の'b'の側の外側にある
		
		// カプセルなので
		Sphere sp(cap.seg.b, cap.r);
		if(intersectSegmentSphere(seg, sp, t))
			return true;	
		return false;	  
	}
	
	// 線分のどちらかの端もしくは両方の円柱底面区間内にある
	float nn = n.dot(n);
	float mn = m.dot(n);
	float a = dd * nn - nd * nd;
	float k = m.dot(m) - cap.r * cap.r;
	float c = dd * k - md * md;
	// 線分が円柱の軸に対して平行に走っているかどうかを判定
	if (fabsf(a) < EPSILON) {	
		if (c > 0.0f) // 'a'および線分は円柱の外側にある
			return false; 
		
		// これで線分が円柱と交差していることが分かったので、どのように交差しているかを調べる
		if (md < 0.0f){ 
			// 線分は'a'の側の底面と交差している
			// カプセルなので
			Sphere sp(cap.seg.a, cap.r);
			intersectSegmentSphere(seg, sp, t);
		}
		else if (md > dd){
			// 線分は'b'の側の底面と交差している
			// カプセルなので
			Sphere sp(cap.seg.b, cap.r);
			intersectSegmentSphere(seg, sp, t);
		}
		else {
			// 'a'は円柱の内側にある
			*t = 0.0f; 
		}
		return true;
	}
	
	float b = dd * mn - nd * md;
	float discr = b * b - a * c;
	if (discr < 0.0f) 
		return false; // 実数解がないので交差はない
	
	*t = (-b - sqrtf(discr)) / a;
	if (*t < 0.0f || *t > 1.0f) 
		return false; // 交差が線分の外側にある
	
	if (md + (*t) * nd < 0.0f) {
		// 円柱の'a'の側の外側で交差
		// カプセルなので
		Sphere sp(cap.seg.a, cap.r);
		return intersectSegmentSphere(seg, sp, t);
	} 
	else if (md + (*t) * nd > dd) {
		// 円柱の'q'の側の外側で交差
		// カプセルなので
		Sphere sp(cap.seg.b, cap.r);
		return intersectSegmentSphere(seg, sp, t);
	}
	// 線分が底面の間の間で交差しているので、tは正しい
	return true;	
}

}
