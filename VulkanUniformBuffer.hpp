#pragma once

#include "Matrix4f.hpp"

struct UniformBufferObject
{
	Matrix4f model;
	Matrix4f view;
	Matrix4f proj;
};
