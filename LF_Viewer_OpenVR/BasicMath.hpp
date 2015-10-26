// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <memory.h>
#include <math.h>
#include <iostream>
#include <cstdio>

// Include GLEW
#include <GL/glew.h>
#include <GL/glut.h>

// Include GLM
#include <glm.hpp>

// Include OpenCL
//#include "LightField_Render_GL.hpp"

#include "textfile.h"

#define M_PI       3.14159265358979323846

using namespace std;

void crossProduct(float *a, float *b, float *res);
void normalize(float *a);
void setIdentityMatrix(float *mat, int size);
void multMatrix(float *a, float *b);
void setTranslationMatrix(float *mat, float x, float y, float z);


void printProgramInfoLog(GLuint obj);
void printShaderInfoLog(GLuint obj);
int printOglError(char *file, int line);

