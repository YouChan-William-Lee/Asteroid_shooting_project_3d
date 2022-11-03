#pragma once

#ifndef _VECTORMATH_H_
#define _VECTORMATH_H_

#include <math.h>

class vec3f {
public:
	float x;
	float y;
	float z;

	vec3f();
	vec3f(float xValue, float yValue, float zValue);
	vec3f(const vec3f& other);
	~vec3f();

	float length() const;
	float distance(const vec3f& other) const;
	float dot(vec3f v);
	vec3f cross(vec3f v);

	vec3f& normalize();

	vec3f operator=(const vec3f& other);
	vec3f operator+(const vec3f& other);
	vec3f operator+(float num);
	vec3f operator-(const vec3f& other);
	vec3f operator/(float num);
	vec3f& operator/=(float num);
	vec3f operator*(float num);
	vec3f& operator*=(float num);
};

#endif