#pragma once

#define FF_VECTOR_2F

#include <math.h>
#include <stdlib.h>
#include <iostream>

struct Vector2f
{
	Vector2f() {} //Default constructor, does nothing, v has undefined values
	Vector2f(const float x, const float y) : v{x, y} {} //Constructs it with x and y.
	//I am not sure if using const references here speeds up stuff.

	inline float x() const { return v[0]; }
	inline float y() const { return v[1]; }

	inline const Vector2f& operator+() const { return *this; }
	inline Vector2f operator-() const { return Vector2f(-v[0], -v[1]); }
	inline float operator[](int i) const { return v[i]; }
	inline float& operator[](int i) { return v[i]; }

	inline Vector2f& operator+=(const Vector2f& v2);
	inline Vector2f& operator-=(const Vector2f& v2); 
	inline Vector2f& operator*=(const Vector2f& v2);
	inline Vector2f& operator/=(const Vector2f& v2); 
	inline Vector2f& operator*=(const float t);
	inline Vector2f& operator/=(const float t); 

	inline float length() const 
	{
		return sqrt(v[0] * v[0] + v[1] * v[1]);
	}

	inline float squared_length() const 
	{
		return v[0] * v[0] + v[1] * v[1];
	}

	inline Vector2f unit() const;
	inline void make_unit_vector();

	float v[2];
};

static inline Vector2f v2f_one()
{
	return
	{
		1.0f, 1.0f
	};
}

static inline Vector2f v2f_zero()
{
	return 
	{
		0.0f, 0.0f
	};
}

inline std::istream& operator>> (std::istream& is, Vector2f& t)
{
	is >> t.v[0] >> t.v[1];
	return is;
}

inline std::ostream& operator<< (std::ostream& os, const Vector2f& t)
{
	os << t.v[0] << " " << t.v[1];
	return os;
}


inline void Vector2f::make_unit_vector()
{
	float k = 1.0f / sqrt(v[0] * v[0] + v[1] * v[1]);

	v[0] *= k;
	v[1] *= k;
}

inline Vector2f Vector2f::unit() const
{
	float k = 1.0f / sqrt(v[0] * v[0] + v[1] * v[1]);

	return Vector2f(v[0] * k, v[1] * k);
}


inline Vector2f operator+(const Vector2f& v1, const Vector2f& v2)
{
	return Vector2f(v1.v[0] + v2.v[0], v1.v[1] + v2.v[1]);
}

inline Vector2f operator-(const Vector2f& v1, const Vector2f& v2)
{
	return Vector2f(v1.v[0] - v2.v[0], v1.v[1] - v2.v[1]);
}

inline Vector2f operator*(const Vector2f& v1, const Vector2f& v2)
{
	return Vector2f(v1.v[0] * v2.v[0], v1.v[1] * v2.v[1]);
}

inline Vector2f operator/(const Vector2f& v1, const Vector2f& v2)
{
	return Vector2f(v1.v[0] / v2.v[0], v1.v[1] / v2.v[1]);
}


inline Vector2f operator*(const Vector2f& v1, const float t)
{
	return Vector2f(v1.v[0] * t, v1.v[1] * t);
}

inline Vector2f operator*(const float t, const Vector2f& v1)
{
	return Vector2f(v1.v[0] * t, v1.v[1] * t);
}

inline Vector2f operator/(const Vector2f& v1, const float t)
{
	return Vector2f(v1.v[0] / t, v1.v[1] / t);
}


inline float v2f_dot(const Vector2f& v1, const Vector2f& v2)
{
	return v1.v[0] * v2.v[0] + v1.v[1] * v2.v[1];
}

inline float v2f_cross(const Vector2f& v1, const Vector2f& v2)
{
	return v1.v[0] * v2.v[1] - (v1.v[1] * v2.v[0]);
}

inline Vector2f& Vector2f::operator+=(const Vector2f& t)
{
	v[0] += t.v[0];
	v[1] += t.v[1];
	return *this;
}

inline Vector2f& Vector2f::operator-=(const Vector2f& t)
{
	v[0] -= t.v[0];
	v[1] -= t.v[1];
	return *this;
}

inline Vector2f& Vector2f::operator*=(const Vector2f& t)
{
	v[0] *= t.v[0];
	v[1] *= t.v[1];
	return *this;
}

inline Vector2f& Vector2f::operator/=(const Vector2f& t)
{
	v[0] /= t.v[0];
	v[1] /= t.v[1];
	return *this;
}

inline Vector2f& Vector2f::operator*=(const float t)
{
	v[0] *= t;
	v[1] *= t;
	return *this;
}

inline Vector2f& Vector2f::operator/=(const float t)
{
	float k = 1.0f / t;

	v[0] *= k;
	v[1] *= k;
	return *this;
}

inline Vector2f translate(const Vector2f& point, const Vector2f& amount)
{
	return point + amount;
}

inline Vector2f rotate(const Vector2f& point, const float theta)
{
	return Vector2f(point.v[0] * cos(theta) - (point.v[1] * sin(theta)),
					point.v[0] * sin(theta) +  point.v[1] * cos(theta)  );
}
