#pragma once 

#ifndef _UNIFIEDHEADER_H_
#define _UNIFIEDHEADER_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <fstream>
#include <iostream>

#if _WIN32
#   include <Windows.h>
#endif
#if __APPLE__
#   include <OpenGL/gl.h>
#   include <OpenGL/glu.h>
#   include <GLUT/glut.h>
#else
#   include <GL/gl.h>
#   include <GL/glu.h>
#   include <GL/glut.h>
#endif

#include <vector>
#include <list>
#include <typeinfo>

using namespace std;

// this space define entire game space size
constexpr float xwidthofspace = 120.0f;
constexpr float yheightofspace = 90.0f;

inline float GetRandom(float amout) {
	float randNum = rand();
	randNum = randNum / static_cast<float>(RAND_MAX) * amout;
	return randNum;
}

#define PI 3.141592653589f

#endif