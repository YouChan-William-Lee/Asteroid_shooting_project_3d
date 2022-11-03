#include "vectormath.h"

vec3f::vec3f() : x(0), y(0), z(0) {}
vec3f::vec3f(float xValue, float yValue, float zValue) : x(xValue), y(yValue), z(zValue) {}
vec3f::vec3f(const vec3f& other) : x(other.x), y(other.y), z(other.z) {}
vec3f::~vec3f() = default;

float vec3f::length() const {
	return static_cast<float>(sqrt((x * x) + (y * y) + (z * z)));
}

float vec3f::distance(const vec3f & other) const {
	vec3f temp(other.x - x, other.y - y, other.z - z);
	return temp.length();
}

float vec3f::dot(vec3f v) {
	return (x * v.x) + (y * v.y) + (z * v.z);
}

vec3f vec3f::cross(vec3f v) {
	vec3f temp = vec3f();

	temp.x = y * v.z - z * v.y;
	temp.y = z * v.x - x * v.z;
	temp.z = x * v.y - y * v.x;
	return temp;
}

vec3f& vec3f::normalize() {
	float templength = length();
	if (templength == 0)
		return *this;
	return (*this) /= templength;
}

vec3f vec3f::operator=(const vec3f & other) {
	x = other.x;
	y = other.y;
	z = other.z;
	return vec3f(x, y, z);
}

vec3f vec3f::operator+(const vec3f & other) {
	vec3f temp;
	temp.x = x + other.x;
	temp.y = y + other.y;
	temp.z = z + other.z;
	return temp;
}

vec3f vec3f::operator+(float num) {
	vec3f temp;
	temp.x = x + num;
	temp.y = y + num;
	temp.z = z + num;
	return temp;
}

vec3f vec3f::operator-(const vec3f & other) {
	vec3f temp;
	temp.x = x - other.x;
	temp.y = y - other.y;
	temp.z = z - other.z;
	return temp;
}

vec3f vec3f::operator/(float num) {
	vec3f temp;
	temp.x = x / num;
	temp.y = y / num;
	temp.z = z / num;
	return temp;
}

vec3f& vec3f::operator/=(float num) {
	x = x / num;
	y = y / num;
	z = z / num;
	return *this;
}

vec3f vec3f::operator*(float num) {
	vec3f temp;
	temp.x = x * num;
	temp.y = y * num;
	temp.z = z * num;
	return temp;
}

vec3f& vec3f::operator*=(float num) {
	x = x * num;
	y = y * num;
	z = z * num;
	return *this;
}