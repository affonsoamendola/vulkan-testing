#pragma once

#define FF_VECTOR_3F

#include <math.h>
#include <stdlib.h>
#include <iostream>

#ifndef FF_VECTOR_2F
#if __has_include("Vector2f.hpp")
#include "Vector2f.hpp"
#endif
#endif

struct Vector3f
{
	Vector3f() {} //Default constructor, does nothing, v has undefined values
	Vector3f(const float x, const float y, const float z) : v{x, y, z} {} //Constructs it with x, y and z values.
	//I am not sure if using const references here speeds up stuff.
#ifdef FF_VECTOR_2F
	Vector3f(Vector2f vector2, const float z) : v{vector2.v[0], vector2.v[1], z} {}
#endif

	inline float x() const { return v[0]; }
	inline float y() const { return v[1]; }
	inline float z() const { return v[2]; }	//Since this can be used both for colors and for xyz spatial coordiantes, have both types of references.
	
	inline float r() const { return v[0]; }
	inline float g() const { return v[1]; }
	inline float b() const { return v[2]; }

	inline const Vector3f& operator+() const { return *this; }
	inline Vector3f operator-() const { return Vector3f(-v[0], -v[1], -v[2]); }
	inline float operator[](int i) const { return v[i]; }
	inline float& operator[](int i) { return v[i]; }

	inline Vector3f& operator+=(const Vector3f& v2);
	inline Vector3f& operator-=(const Vector3f& v2); 
	inline Vector3f& operator*=(const Vector3f& v2);
	inline Vector3f& operator/=(const Vector3f& v2); 
	inline Vector3f& operator*=(const float t);
	inline Vector3f& operator/=(const float t); 

	inline float length() const 
	{
		return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	}

	inline float squared_length() const 
	{
		return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	}

	inline Vector3f unit() const;
	inline void make_unit_vector();

#ifdef FF_VECTOR_2F
	inline Vector2f to_vector2f() const { return Vector2f(v[0], v[1]);}
#endif

	float v[3];
};

static inline Vector3f v3f_one()
{
	return
	{
		1.0f, 1.0f, 1.0f
	};
}

static inline Vector3f v3f_zero()
{
	return 
	{
		0.0f, 0.0f, 0.0f
	};
}

inline std::istream& operator>> (std::istream& is, Vector3f& t)
{
	is >> t.v[0] >> t.v[1] >> t.v[2];
	return is;
}

inline std::ostream& operator<< (std::ostream& os, const Vector3f& t)
{
	os << t.v[0] << " " << t.v[1] << " " << t.v[2];
	return os;
}


inline void Vector3f::make_unit_vector()
{
	float k = 1.0f / sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

	v[0] *= k;
	v[1] *= k;
	v[2] *= k;
}

inline Vector3f Vector3f::unit() const
{
	float k = 1.0f / sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

	return Vector3f(v[0] * k, v[1] * k, v[2] * k);
}


inline Vector3f operator+(const Vector3f& v1, const Vector3f& v2)
{
	return Vector3f(v1.v[0] + v2.v[0], v1.v[1] + v2.v[1], v1.v[2] + v2.v[2]);
}

inline Vector3f operator-(const Vector3f& v1, const Vector3f& v2)
{
	return Vector3f(v1.v[0] - v2.v[0], v1.v[1] - v2.v[1], v1.v[2] - v2.v[2]);
}

inline Vector3f operator*(const Vector3f& v1, const Vector3f& v2)
{
	return Vector3f(v1.v[0] * v2.v[0], v1.v[1] * v2.v[1], v1.v[2] * v2.v[2]);
}

inline Vector3f operator/(const Vector3f& v1, const Vector3f& v2)
{
	return Vector3f(v1.v[0] / v2.v[0], v1.v[1] / v2.v[1], v1.v[2] / v2.v[2]);
}


inline Vector3f operator*(const Vector3f& v1, const float t)
{
	return Vector3f(v1.v[0] * t, v1.v[1] * t, v1.v[2] * t);
}

inline Vector3f operator*(const float t, const Vector3f& v1)
{
	return Vector3f(v1.v[0] * t, v1.v[1] * t, v1.v[2] * t);
}

inline Vector3f operator/(const Vector3f& v1, const float t)
{
	return Vector3f(v1.v[0] / t, v1.v[1] / t, v1.v[2] / t);
}


inline float v3f_dot(const Vector3f& v1, const Vector3f& v2)
{
	return v1.v[0] * v2.v[0] + v1.v[1] * v2.v[1] + v1.v[2] * v2.v[2];
}

inline Vector3f v3f_cross(const Vector3f& v1, const Vector3f& v2)
{
	return Vector3f (	(v1.v[1] * v2.v[2] - v1.v[2] * v2.v[1]),
					   -(v1.v[0] * v2.v[2] - v1.v[2] * v2.v[0]),
					    (v1.v[0] * v2.v[1] - v1.v[1] * v2.v[0])   );
}

inline Vector3f& Vector3f::operator+=(const Vector3f& t)
{
	v[0] += t.v[0];
	v[1] += t.v[1];
	v[2] += t.v[2];
	return *this;
}

inline Vector3f& Vector3f::operator-=(const Vector3f& t)
{
	v[0] -= t.v[0];
	v[1] -= t.v[1];
	v[2] -= t.v[2];
	return *this;
}

inline Vector3f& Vector3f::operator*=(const Vector3f& t)
{
	v[0] *= t.v[0];
	v[1] *= t.v[1];
	v[2] *= t.v[2];
	return *this;
}

inline Vector3f& Vector3f::operator/=(const Vector3f& t)
{
	v[0] /= t.v[0];
	v[1] /= t.v[1];
	v[2] /= t.v[2];
	return *this;
}

inline Vector3f& Vector3f::operator*=(const float t)
{
	v[0] *= t;
	v[1] *= t;
	v[2] *= t;
	return *this;
}

inline Vector3f& Vector3f::operator/=(const float t)
{
	float k = 1.0f / t;

	v[0] *= k;
	v[1] *= k;
	v[2] *= k;
	return *this;
}

inline Vector3f v3f_translate(const Vector3f& point, const Vector3f& amount)
{
	return point + amount;
}

inline Vector3f v3f_rotateX(const Vector3f& point, const float theta)
{
	return Vector3f(point.v[0], 
					point.v[1] * cos(theta) - point.v[2] * sin(theta), 
					point.v[2] * cos(theta) + point.v[1] * sin(theta)	);
}

inline Vector3f v3f_rotateY(const Vector3f& point, const float theta)
{
	return Vector3f(point.v[0] * cos(theta) - point.v[2] * sin(theta), 
					point.v[1], 
					point.v[2] * cos(theta) + point.v[0] * sin(theta));
}

inline Vector3f v3frotateZ(const Vector3f& point, const float theta)
{
	return Vector3f(point.v[0] * cos(theta) - point.v[1] * sin(theta), 
					point.v[1] * cos(theta) + point.v[0] * sin(theta), 
					point.v[2]);
}

#if __has_include("Color.hpp")
#include "Color.hpp"

inline Color v3f_to_color (const Vector3f& t, const unsigned char a)
{
	return Color(	static_cast<unsigned char>(t.v[0] * 255.0f), 
					static_cast<unsigned char>(t.v[1] * 255.0f),
					static_cast<unsigned char>(t.v[2] * 255.0f),
					a);
} 

#endif