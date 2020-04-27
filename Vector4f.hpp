#pragma once

#define FF_VECTOR_4F

#include <math.h>
#include <stdlib.h>
#include <iostream>

#ifndef FF_VECTOR_3F
#if __has_include("Vector3f.hpp")
#include "Vector3f.hpp"
#endif
#endif

#ifndef FF_VECTOR_2F
#if __has_include("Vector2f.hpp")
#include "Vector2f.hpp"
#endif
#endif

struct Vector4f
{
	Vector4f() {} //Default constructor, does nothing, v has undefined values
	Vector4f(const float x, const float y, const float z, const float w) : v{x, y, z, w} {} //Constructs it with x, y and z values.
#ifdef FF_VECTOR_3F
	Vector4f(const Vector3f& vector3, const float w) : v{vector3.v[0], vector3.v[1], vector3.v[2], w} {}
#endif
#ifdef FF_VECTOR_2F
	Vector4f(const Vector2f& vector2, const float z, const float w) : v{vector2.v[0], vector2.v[1], z, w} {}
#endif

	inline float x() const { return v[0]; }
	inline float y() const { return v[1]; }
	inline float z() const { return v[2]; }	//Since this can be used both for colors and for xyz spatial coordiantes, have both types of references.
	inline float w() const { return v[3]; }

	inline float r() const { return v[0]; }
	inline float g() const { return v[1]; }
	inline float b() const { return v[2]; }
	inline float a() const { return v[2]; }

	inline const Vector4f& operator+() const { return *this; }
	inline Vector4f operator-() const { return Vector4f(-v[0], -v[1], -v[2], -v[3]); }
	inline float operator[](int i) const { return v[i]; }
	inline float& operator[](int i) { return v[i]; }

	inline Vector4f& operator+=(const Vector4f& v2);
	inline Vector4f& operator-=(const Vector4f& v2); 
	inline Vector4f& operator*=(const Vector4f& v2);
	inline Vector4f& operator/=(const Vector4f& v2); 
	inline Vector4f& operator*=(const float t);
	inline Vector4f& operator/=(const float t); 

	inline float length() const 
	{
		return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);
	}

	inline float squared_length() const 
	{
		return v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3];
	}

	inline Vector4f unit() const;
	inline void make_unit_vector();

#ifdef FF_VECTOR_3F 
	inline Vector3f to_vector3f_truncate() const { return Vector3f(v[0], v[1], v[2]); }
	inline Vector3f to_vector3f_homogenize() const 
	{ 
		if(v[3] != 1.0f)
		{
			return Vector3f(v[0]/v[3], v[1]/v[3], v[2]/v[3]);
		}
		else return Vector3f(v[0], v[1], v[2]);
		 
	}
#endif
#ifdef FF_VECTOR_2F 
	inline Vector2f to_vector2f() const { return Vector2f(v[0], v[1]); }
#endif

	float v[4];
};

static inline Vector4f v4f_one()
{
	return
	{
		1.0f, 1.0f, 1.0f, 1.0f
	};
}

static inline Vector4f v4f_zero()
{
	return 
	{
		0.0f, 0.0f, 0.0f, 0.0f
	};
}

inline std::istream& operator>> (std::istream& is, Vector4f& t)
{
	is >> t.v[0] >> t.v[1] >> t.v[2] >> t.v[3];
	return is;
}

inline std::ostream& operator<< (std::ostream& os, const Vector4f& t)
{
	os << t.v[0] << " " << t.v[1] << " " << t.v[2] << " " << t.v[3];
	return os;
}


inline void Vector4f::make_unit_vector()
{
	float k = 1.0f / sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);

	v[0] *= k;
	v[1] *= k;
	v[2] *= k;
	v[3] *= k;
}

inline Vector4f Vector4f::unit() const
{
	float k = 1.0f / sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3]);

	return Vector4f(v[0] * k, v[1] * k, v[2] * k, v[3] * k);
}


inline Vector4f operator+(const Vector4f& v1, const Vector4f& v2)
{
	return Vector4f(v1.v[0] + v2.v[0], v1.v[1] + v2.v[1], v1.v[2] + v2.v[2], v1.v[3] + v2.v[3]);
}

inline Vector4f operator-(const Vector4f& v1, const Vector4f& v2)
{
	return Vector4f(v1.v[0] - v2.v[0], v1.v[1] - v2.v[1], v1.v[2] - v2.v[2], v1.v[3] - v2.v[3]);
}

inline Vector4f operator*(const Vector4f& v1, const Vector4f& v2)
{
	return Vector4f(v1.v[0] * v2.v[0], v1.v[1] * v2.v[1], v1.v[2] * v2.v[2], v1.v[3] * v2.v[3]);
}

inline Vector4f operator/(const Vector4f& v1, const Vector4f& v2)
{
	return Vector4f(v1.v[0] / v2.v[0], v1.v[1] / v2.v[1], v1.v[2] / v2.v[2], v1.v[3] / v2.v[3]);
}


inline Vector4f operator*(const Vector4f& v1, const float t)
{
	return Vector4f(v1.v[0] * t, v1.v[1] * t, v1.v[2] * t, v1.v[3] * t);
}

inline Vector4f operator*(const float t, const Vector4f& v1)
{
	return Vector4f(v1.v[0] * t, v1.v[1] * t, v1.v[2] * t, v1.v[3] * t);
}

inline Vector4f operator/(const Vector4f& v1, const float t)
{
	return Vector4f(v1.v[0] / t, v1.v[1] / t, v1.v[2] / t, v1.v[3] / t);
}


inline float v4f_dot(const Vector4f& v1, const Vector4f& v2)
{
	return v1.v[0] * v2.v[0] + v1.v[1] * v2.v[1] + v1.v[2] * v2.v[2] + v1.v[3] * v2.v[3];
}
/*
inline Vector4f cross(const Vector4f& v1, const Vector4f& v2)
{
	TODO: Implement cross for vector4
}
*/
inline Vector4f& Vector4f::operator+=(const Vector4f& t)
{
	v[0] += t.v[0];
	v[1] += t.v[1];
	v[2] += t.v[2];
	v[3] += t.v[3];
	return *this;
}

inline Vector4f& Vector4f::operator-=(const Vector4f& t)
{
	v[0] -= t.v[0];
	v[1] -= t.v[1];
	v[2] -= t.v[2];
	v[3] -= t.v[3];
	return *this;
}

inline Vector4f& Vector4f::operator*=(const Vector4f& t)
{
	v[0] *= t.v[0];
	v[1] *= t.v[1];
	v[2] *= t.v[2];
	v[3] *= t.v[3];
	return *this;
}

inline Vector4f& Vector4f::operator/=(const Vector4f& t)
{
	v[0] /= t.v[0];
	v[1] /= t.v[1];
	v[2] /= t.v[2];
	v[3] /= t.v[3];
	return *this;
}

inline Vector4f& Vector4f::operator*=(const float t)
{
	v[0] *= t;
	v[1] *= t;
	v[2] *= t;
	v[3] *= t;
	return *this;
}

inline Vector4f& Vector4f::operator/=(const float t)
{
	float k = 1.0f / t;

	v[0] *= k;
	v[1] *= k;
	v[2] *= k;
	v[3] *= k;
	return *this;
}

inline Vector4f v4f_translate(const Vector4f& point, const Vector4f& amount)
{
	return point + amount;
}

#if __has_include("Color.hpp")
#include "Color.hpp"

inline Color v4f_to_color (const Vector4f& t)
{
	return Color(	static_cast<unsigned char>(t.v[0] * 255.0f), 
					static_cast<unsigned char>(t.v[1] * 255.0f),
					static_cast<unsigned char>(t.v[2] * 255.0f),
					static_cast<unsigned char>(t.v[3] * 255.0f));
} 

#endif