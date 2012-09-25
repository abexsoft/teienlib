#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <iostream>
#include <sstream>
#include <math.h>
#include <stdlib.h>
using namespace std;

namespace sul
{

template <class T>
struct Vector3D{
	T x;
	T y;
	T z;
	
	Vector3D() { x = 0; y = 0; z = 0; }
	Vector3D(const Vector3D& pt) { x = pt.x; y = pt.y; z = pt.z; }
	Vector3D(T in_x, T in_y, T in_z) { x = in_x; y = in_y; z = in_z; }
	
	void set(T ix, T iy, T iz) { x = ix; y = iy; z = iz; }
	void set(const Vector3D &pt) { x = pt.x; y = pt.y; z = pt.z; }
	
	Vector3D operator+(const Vector3D &pt) const 
	{ 
		return Vector3D (x + pt.x, y + pt.y, z + pt.z); 
	}
	Vector3D operator-(const Vector3D &pt) const 
	{ 
		return Vector3D (x - pt.x, y - pt.y, z - pt.z); 
	}
	Vector3D operator-() const { return Vector3D(-x, -y, -z); }
	Vector3D &operator+=(const Vector3D &pt) 
	{ 
		x += pt.x; y += pt.y; z += pt.z; return *this; 
	}
	Vector3D &operator-=(const Vector3D &pt) 
	{ 
		x -= pt.x; y -= pt.y; z -= pt.z; return *this; 
	}
	Vector3D operator*(const T f)	{ return Vector3D (x * f, y * f, z *f); }
	Vector3D &operator*=(const T f) { x *= f; y *= f; z *= f; return *this; }
	
	Vector3D operator/(const T f)	{ return Vector3D (x / f, y / f, z / f); }
	Vector3D &operator/=(const T f) { x /= f; y /= f; z /= f; return *this; }
	
	Vector3D &operator=(const Vector3D &pt) { x = pt.x; y = pt.y; z = pt.z; return *this; }
	bool operator==(const Vector3D &pt)	 { return x == pt.x && y == pt.y && z == pt.z; }
	
	T len() const { return (T) sqrt(x * x + y * y + z * z); }
	T lenSquared() const { return x * x + y * y + z * z; }
	bool normalize() 
	{
		T l = len();
		if (l == 0) { x = 0; y = 0; z = 0; return false;}
		else { l = 1 / l; x *= l; y *= l; z *= l; return true;}
	}
	bool normalize(float newLen) 
	{
		T l = len();
		if (l == 0) { x = 0; y = 0; z = 0; return false;}
		else { l = newLen / l; x *= l; y *= l; z *= l; return true;}
	}
	void scaleFloorDiv(float scaleFactor, float divFactor)
	{
		x = (T) floor(x * scaleFactor + 0.5) * divFactor;
		y = (T) floor(y * scaleFactor + 0.5) * divFactor;
		z = (T) floor(z * scaleFactor + 0.5) * divFactor;
	}
	T dot(const Vector3D &p) const { return x * p.x + y * p.y + z * p.z; }
	void read(const char **argv) 
	{
		x = (T) atof(argv[0]);
		y = (T) atof(argv[1]);
		z = (T) atof(argv[2]);
	}
	void printPos()
	{
		cout << "(" << x << ", " << y << ", " << z << ")";
	}
	std::string toString()
	{
		std::ostringstream oss;
		oss << "(" << x << ", " << y << ", " << z << ")";
		return oss.str();
	}
};

}
#endif // VECTOR3D_H
