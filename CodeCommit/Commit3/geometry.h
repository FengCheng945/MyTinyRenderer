#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include<iostream>
#include <cmath>
#include<cassert>

template<typename T, size_t LEN> struct Vector {};
template<typename T> struct Vector<T, 2>
{
	union
	{
		struct { T u, v; };
		struct { T x, y; };
		T raw[2];
	};
	Vector() : x(0), y(0) {}
	Vector(T x0, T y0) : x(x0), y(y0) {}
	Vector(const Vector<T, 2>& v)
	{
		x = v.x;
		y = v.y;
	}
	void operator=(const Vector<T,2>& v)
	{
		x = v.x;
		y = v.y;
	}
	inline Vector<T, 2> cwiseProduct(const Vector<T, 2>& v)
	{	//Vector * Vector
		return Vector<T, 2>(x * v.x, y * v.y);
	}

	T& operator[](const size_t i) { assert(i < 2); return i <= 0 ? x : y; }
	const T& operator[](const size_t i) const { assert(i < 2); return i <= 0 ? x : y; }

	template <typename > friend std::ostream& operator<<(std::ostream& s, Vector<T, 2>& v);
};
template<typename T> struct Vector<T, 3>
{
	union
	{
		struct { T x, y, z; };
		struct { T ivert, iuv, inorm; };
		T raw[3];
	};

	Vector() : x(0), y(0), z(0) {};
	Vector(T x0, T y0, T z0) : x(x0), y(y0), z(z0) {}
	Vector(const Vector<T, 3>& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}
	void operator=(const Vector<T, 3>& v) { 
		x = v.x;
		y = v.y;
		z = v.z;
	}
	T& operator[](const size_t i) { assert(i < 3); return i <= 0 ? x : (1 == i ? y : z); }
	const T& operator[](const size_t i) const { assert(i < 3); return i <= 0 ? x : (1 == i ? y : z); }

	inline Vector<T, 3> cwiseProduct(const Vector<T, 3>& v) const
	{	//Vector * Vector
		return Vector<T, 3>(x * v.x, y * v.y, z * v.z);
	}
	inline Vector<T, 3> cross(const Vector<T, 3>& v) const { return Vector<T, 3>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }

	float norm() const { return std::sqrt(x * x + y * y + z * z); }
	Vector<T, 3>& normalize(T l = 1) { *this = (*this) * (l / norm()); return *this; }
	template <typename > friend std::ostream& operator<<(std::ostream& s, Vector<T, 3>& v);
};
template<typename T, size_t LEN>
inline Vector<T, LEN> operator+(Vector<T, LEN> res, const Vector<T, LEN>& v)
{	//
	for (size_t i = LEN; i--; res[i] += v[i]);
	return res;
};
template<typename T, size_t LEN>
inline Vector<T, LEN> operator-(Vector<T, LEN> res, const Vector<T, LEN>& v)
{
	for (size_t i = LEN; i--; res[i] -= v[i]);
	return res;
};
template<typename T, size_t LEN>
inline Vector<T, LEN> operator*(Vector<T, LEN> res, float f)
{   //broadcast f
	for (size_t i = LEN; i--; res[i] *= f);
	return res;
};
template<typename T, size_t LEN>
inline Vector<T, LEN> operator*(float f, Vector<T, LEN> res)
{   //broadcast f
	for (size_t i = LEN; i--; res[i] *= f);
	return res;
};
template<typename T, size_t LEN>
inline T operator*(const Vector<T, LEN>& res, const Vector<T, LEN>& v)
{	// Vector * Vector and + return T;
	T t = 0;
	for (size_t i = LEN; i--; t += res[i] * v[i]);
	return t;
};
template<typename T, size_t LEN>
inline Vector<T, LEN> operator/(Vector<T, LEN> res, float f)
{   //broadcast f
	for (size_t i = LEN; i--; res[i] /= f);
	return res;
};
template<typename T>
std::ostream& operator<<(std::ostream& s, Vector<T, 2>& v)
{
	s << "(" << v.x << ", " << v.y << ")\n";
	return s;
}
template<typename T>
std::ostream& operator<<(std::ostream& s, Vector<T, 3>& v)
{
	s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
	return s;
}

typedef Vector<int, 2> Vector2i;
typedef Vector<float, 2> Vector2f;
typedef Vector<int, 3> Vector3i;
typedef Vector<float, 3> Vector3f;

#endif //__GEOMETRY_H__
