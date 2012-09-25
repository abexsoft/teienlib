#ifndef SHAPE_H
#define SHAPE_H

#include "Vector3D.h"

using namespace sul;

namespace scl{

struct Shape
{
	Shape();
	virtual ~Shape(){}
	virtual void setPos(Vector3D<float> aPos){ cerr << "Error: Shape::setPos()" << endl;}
	
	virtual const char* getClassName() const = 0;
	
	// aDir & localPosにより位置が決まる
	//virtual void setPos(Vector3D<float> aPos, Vector3D<float> aDir){ cerr << "Error: Shape::setPos()" << endl;}
	virtual void printInfo() const { cout << "struct: Shape" << endl; }
	
	/// relative position to Entity
	/**
	 * localPosは、Entityのpos(0, 0, 0), dir(0, -1, 0)の時の位置にしたいが
	 * dirに関しては現在未対応
	 */
	Vector3D<float> localPos;
};

struct AABB : public Shape
{
	AABB(){}
	AABB(Vector3D<float> aMin, Vector3D<float> aMax) :	min(aMin), max(aMax){}
	~AABB(){}
	Vector3D<float> getCenter() const
	{
		return (min + max) / 2;
	}
	Vector3D<float> getRadius() const
	{
		return (max - min) / 2;
	}
	
	void setPos(Vector3D<float> aCenter, Vector3D<float> aRad)
	{
		aCenter = aCenter + localPos;
		min = aCenter - aRad;
		max = aCenter + aRad;
	}
	void setPos(Vector3D<float> aCenter)
	{
		setPos(aCenter, getRadius());
	}
	void setRadius(Vector3D<float> aRad)
	{
		setPos(getCenter(), aRad);
	}
	
	const char* getClassName() const { return "AABB"; }
	
	void printInfo() const 
	{
		Vector3D<float> vect = getCenter();
		cout << "** struct: AABB [" << this << "]" << endl;
		cout << "center: (" << vect.x << ", " << vect.y << ", " << vect.z <<")" << endl;
		vect = getRadius();
		cout << "radius: (" << vect.x << ", " << vect.y << ", " << vect.z <<")" << endl;
	}
	
	Vector3D<float> min; 
	Vector3D<float> max; 
};

struct Sphere : public Shape
{
	Sphere(){};
	~Sphere(){};
	Sphere(Vector3D<float> aCen, float aRad)
	{
		c = aCen + localPos; r = aRad;
	}
	
	float getRadius() const { return r; }
	Vector3D<float> getCenter() const { return c; }
	
	void setPos(Vector3D<float> aCenter)
	{
		c = aCenter + localPos;
	}
	void setRadius(float aRad)
	{
		r = aRad;
	}
	void setPos(Vector3D<float> aCenter, float aRad)
	{
		setPos(aCenter);
		setRadius(aRad);
	}
	
	const char* getClassName() const { return "Sphere"; }
	
	void printInfo() const 
	{
		Vector3D<float> vect = getCenter();
		cout << "** struct: Sphere [" << this << "]" << endl;
		cout << "center: (" << vect.x << ", " << vect.y << ", " << vect.z <<")" << endl;
		cout << "radius: " << r << endl;
	}
	
	Vector3D<float> c; // center
	float			r;
};

/**
 *	 n.dot(x) - d = 0;
 *	 nx * x + ny * y + nz * z - d = 0;
 */
struct Plane : public Shape
{
	Plane(){};
	Plane(Vector3D<float> aN, float aD) : n(aN), d(aD) {};
	~Plane(){};
	
	const char* getClassName() const { return "Plane"; }
	
	void printInfo() const 
	{
		cout << "** struct: Plane [" << this << "]" << endl;
		cout << "normal: (" << n.x << ", " << n.y << ", " << n.z <<")" << endl;
		cout << "d: " << d << endl;
	}
	
	Vector3D<float> n;	// d = n.dot(x) 
	float			d;
};


/*
 *
 * 以下、collision演算用にしか使われていない
 *
 */

struct Segment : public Shape
{
	Segment(){};
	Segment(Vector3D<float> aA, Vector3D<float> aB)
	{
		a = aA; b = aB;
	}
	~Segment(){};
	
	const char* getClassName() const { return "Segment"; }
	
	void printInfo() const 
	{
		cout << "** struct: Segment [" << this << "]" << endl;
		cout << "a: (" << a.x << ", " << a.y << ", " << a.z <<")" << endl;
		cout << "b: (" << b.x << ", " << b.y << ", " << b.z <<")" << endl;
	}
	
	Vector3D<float> a;
	Vector3D<float> b;
};

struct Capsule : public Shape
{
	Capsule(){};
	~Capsule(){};
	
	const char* getClassName() const { return "Capsule"; }
	
	void printInfo() const 
{
		cout << "** struct: Capsule [" << this << "]" << endl;
		cout << "segment: a(" << seg.a.x << ", " << seg.b.y << ", " << seg.b.z <<"), " 
			 << "b(" << seg.b.x << ", " << seg.b.y << ", " << seg.b.z << ")" << endl;
		cout << "radius: " << r << endl;
	}
	
	Segment seg;
	float	r;
};

}
#endif
