#pragma once

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <float.h>

#if __has_include("Vector3f.hpp")
#include "Vector3f.hpp"
#endif

#if __has_include("Vector4f.hpp")
#include "Vector4f.hpp"
#endif

/* Column Major storage
0  4  8  12
1  5  9  13
2  6  10 14
3  7  11 15
*/

struct Matrix4f
{
	float v[16];

	Matrix4f() {} //Default constructor, does nothing, v has undefined values
	
	Matrix4f(const float value[16]) 
	{
		memcpy(v, value, sizeof(value[0]) * 16);
	}

	Matrix4f(std::initializer_list<float> init) 
	{
		std::copy(init.begin(), init.end(), v);
	}

	inline const Matrix4f& operator+() const { return *this; }
	inline Matrix4f operator-() const;

	inline float operator[](int i) const { return v[i]; }
	inline float& operator[](int i) { return v[i]; }

	inline float get(int col, int line) const { return v[col * 4 + line]; }
	inline float& get(int col, int line) { return v[col * 4 + line]; }

	inline float* get_col(int col) {return &v[col * 4]; }

	inline Matrix4f& operator+=(const Matrix4f& m);
	inline Matrix4f& operator-=(const Matrix4f& m);
	inline Matrix4f& operator*=(const float t);
	inline Matrix4f& operator/=(const float t); 

	inline Matrix4f& operator*=(const Matrix4f& m);
	
	inline Matrix4f transpose();
	//inline Matrix4f invert();
	
	inline float trace();
	inline float determinant();
};

static inline Matrix4f m4f_identity()
{
	return
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
}

static inline Matrix4f m4f_zero()
{
	return 
	{
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f
	};
}


inline std::ostream& operator<< (std::ostream& os, const Matrix4f& t)
{
	for(int l = 0; l < 4; l++)
	{
		os << t.get(0, l) << "\t" << t.get(1, l) << "\t" << t.get(2, l) << "\t" << t.get(3, l) << "\n";
	}	
	return os;
}

inline Matrix4f Matrix4f::operator-() const
{
	Matrix4f matrix;

	for(int i = 0; i < 16; i++)
	{
		matrix.v[i] = -v[i];
	}

	return matrix;
}

inline Matrix4f& Matrix4f::operator+=(const Matrix4f& m)
{
	for(int i = 0; i < 16; i++)
	{
		v[i] += m.v[i];
	}

	return *this;
}

inline Matrix4f& Matrix4f::operator-=(const Matrix4f& m)
{
	for(int i = 0; i < 16; i++)
	{
		v[i] -= m.v[i];
	}

	return *this;
}

inline Matrix4f& Matrix4f::operator*=(const float t)
{
	for(int i = 0; i < 16; i++)
	{
		v[i] *= t;
	}

	return *this;
}

inline Matrix4f& Matrix4f::operator/=(const float t)
{
	for(int i = 0; i < 16; i++)
	{
		v[i] /= t;
	}

	return *this;
}

inline Matrix4f Matrix4f::transpose()
{
	Matrix4f matrix;

	for(int i = 0; i < 16; i++)
	{
		matrix.v[i] = v[(i*4)%16 + i/4]; //I swear this is a transpose, took me a while to figure this one out but it works.
	}

	return matrix;
}

/*
inline Matrix4f invert()
{

	//TODO: IMPLEMENT MATRIX INVERSION.
}
*/

inline float Matrix4f::trace()
{
	float trace = 0.0f;

	for(int i = 0; i < 4; i++)
	{
		trace += v[i*5]; //[i*4 + i] = [i*(4+1)] = [i*5]
	}

	return trace;
}

inline float Matrix4f::determinant()
{
	float det = 1.0f;
	//Jesus christ this is lazy.
	//TODO: A generic version FOR THE LOVE OF *GOD*
	det = 	v[0] * v[5] * v[10] * v[15] + 
			v[4] * v[9] * v[14] * v[3]  +
			v[8] * v[13]* v[2]  * v[7]  +
			v[12]* v[1] * v[6]  * v[11] -
		   (v[0] * v[13]* v[10] * v[7]  +
			v[4] * v[1] * v[14] * v[11] +
			v[8] * v[5] * v[2]  * v[15] +
			v[12]* v[9] * v[6]  * v[3]);
   
   	return det;
}

inline Matrix4f operator+(const Matrix4f& m1, const Matrix4f& m2)
{
	Matrix4f matrix;

	for(int i = 0; i < 16; i++)
	{
		matrix.v[i] = m1.v[i] + m2.v[i];
	}

	return matrix;
}

inline Matrix4f operator-(const Matrix4f& m1, const Matrix4f& m2)
{
	Matrix4f matrix;

	for(int i = 0; i < 16; i++)
	{
		matrix.v[i] = m1.v[i] - m2.v[i];
	}

	return matrix;
}

inline Matrix4f operator*(const Matrix4f& m1, const float t)
{
	Matrix4f matrix;

	for(int i = 0; i < 16; i++)
	{
		matrix.v[i] = m1.v[i] * t;
	}

	return matrix;
}

inline Matrix4f operator*(const float t, const Matrix4f& m1 )
{
	Matrix4f matrix = m1 * t;

	return matrix;
}

inline Matrix4f operator/(const Matrix4f& m1, const float t)
{
	Matrix4f matrix;

	for(int i = 0; i < 16; i++)
	{
		matrix.v[i] = m1.v[i] / t;
	}

	return matrix;
}

inline Matrix4f operator*(const Matrix4f& m1, const Matrix4f& m2)
{
	Matrix4f matrix = m4f_zero();

	for(int c = 0; c < 4; c++)
	{
		for(int l = 0; l < 4; l++)
		{	
			for(int i = 0; i < 4; i++)
			{	
				matrix.v[c*4 + l] += m1.v[i*4 + l] * m2.v[c*4 + i];
			}
		}
	}

	return matrix;
}

inline Matrix4f& Matrix4f::operator*=(const Matrix4f& m1)
{
	Matrix4f matrix = *this * m1;

	memcpy(v, matrix.v, sizeof(v[0]) * 16);

	return *this;
}

static inline Matrix4f m4f_perspective(float fov_y_rad, float aspect, float z_near, float z_far)
{
	float const tan_half_fov_y = tan(fov_y_rad / 2.0f);

	Matrix4f result = m4f_zero();

	result.get(0, 0) = 1.0f / (aspect * tan_half_fov_y);
	result.get(1, 1) = -1.0f / (tan_half_fov_y);
	result.get(2, 2) = (z_far) / (z_far - z_near);
	result.get(2, 3) = 1.0f;
	result.get(3, 2) = -(z_far * z_near) / (z_far - z_near);

	return result;
}

#ifdef FF_VECTOR_3F
	static inline Matrix4f m4f_scale(const Vector3f& amount)
	{
		Matrix4f scale = m4f_identity();

		scale.get(0, 0) = amount.x();
		scale.get(1, 1) = amount.y();
		scale.get(2, 2) = amount.z();

		return scale;
	}

	static inline Matrix4f m4f_translate(const Vector3f& amount)
	{
		Matrix4f tran = m4f_identity();

		tran.get(3, 0) = amount.x();
		tran.get(3, 1) = amount.y();
		tran.get(3, 2) = amount.z();

		return tran;
	}

	static inline Matrix4f m4f_rotate(const Matrix4f& m, float angle, const Vector3f& t_axis)
	{
		float c = cos(angle);
		float s = sin(angle);

		Vector3f axis = t_axis.unit();
		Vector3f temp = axis * (1.0f - c);

		Matrix4f rotate = m4f_identity();

		rotate.get(0, 0) = c + temp[0] * axis[0];
		rotate.get(0, 1) = temp[0] * axis[1] + s * axis[2];
		rotate.get(0, 2) = temp[0] * axis[2] - s * axis[1];

		rotate.get(1, 0) = temp[1] * axis[0] - s * axis[2];
		rotate.get(1, 1) = c + temp[1] * axis[1];
		rotate.get(1, 2) = temp[1] * axis[2] + s * axis[0];

		rotate.get(2, 0) = temp[2] * axis[0] + s * axis[1];
		rotate.get(2, 1) = temp[2] * axis[1] - s * axis[0];
		rotate.get(2, 2) = c + temp[2] * axis[2];

		return rotate * m;
	}

	static inline Matrix4f m4f_look_at(const Vector3f& t_from, const Vector3f& t_to, const Vector3f& t_up)
	{
		Vector3f const forward = (t_to - t_from).unit();
		Vector3f new_up;

		if(fabs(forward.x()) < FLT_EPSILON && fabs(forward.z()) < FLT_EPSILON)
    	{
	        // forward vector is pointing +Y axis
	        if(forward.y() > 0)
	            new_up = Vector3f(0., 0., -1.);
	        // forward vector is pointing -Y axis
	        else
	            new_up = Vector3f(0., 0., 1.);
    	}
    	else
	    {
	        new_up = Vector3f(0, 1, 0);
	    }

		Vector3f const left = v3f_cross(new_up, forward).unit();
		Vector3f const up = v3f_cross(forward, left);

		Matrix4f result = m4f_identity();

		result.get(0, 0) = left[0];
		result.get(0, 1) = left[1];
		result.get(0, 2) = left[2];
		result.get(1, 0) = up[0];
		result.get(1, 1) = up[1];
		result.get(1, 2) = up[2];
		result.get(2, 0) = forward[0];
		result.get(2, 1) = forward[1];
		result.get(2, 2) = forward[2];

		return result;
	}

	#ifdef FF_VECTOR_4F
		inline Vector4f operator*(const Matrix4f& m, const Vector4f& v)
		{
			Vector4f result = v4f_zero();

			for(int l = 0; l < 4; l++)
			{
				for(int c = 0; c < 4; c++)
				{
					result.v[l] += v.v[c] * m.get(c, l);
				}
			}

			return result;
		}

		inline Vector3f operator*(const Matrix4f& m, const Vector3f& v)
		{
			Vector4f result = m * Vector4f(v, 1.0f);

			return result.to_vector3f_homogenize();
		}
	#endif
#endif