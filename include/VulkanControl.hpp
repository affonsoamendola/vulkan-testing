#pragma once

#include "Matrix4f.hpp"

//Remember to check for aligment issues.
/*
    Scalars have to be aligned by N (= 4 bytes given 32 bit floats).
    A vec2 must be aligned by 2N (= 8 bytes)
    A vec3 or vec4 must be aligned by 4N (= 16 bytes)
    A nested structure must be aligned by the base alignment of its members rounded up to a multiple of 16.
    A mat4 matrix must have the same alignment as a vec4.

	use alignas(bytes) to align things properly
*/

struct UniformBufferObject
{
	Matrix4f model;
	Matrix4f view;
	Matrix4f proj;
};
