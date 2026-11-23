//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#ifndef GLUTILS_HPP
#define GLUTILS_HPP

#include <SDL3/SDL_opengl.h>

// Replacements for the parts of GLU that jigtest uses. GLU is deprecated or missing on some
// platforms, and these are simple enough to do ourselves.

// Sets up a perspective projection, multiplying the current matrix (as gluPerspective)
void SetPerspective(double fovYDegrees, double aspect, double zNear, double zFar);

// Sets up a view transform, multiplying the current matrix (as gluLookAt)
void LookAt(double eyeX, double eyeY, double eyeZ,
            double centreX, double centreY, double centreZ,
            double upX, double upY, double upZ);

// Maps window coordinates to object coordinates (as gluUnProject). Returns false if the matrices
// can't be inverted.
bool UnProject(double winX, double winY, double winZ,
               const double modelView[16], const double projection[16], const GLint viewport[4],
               double * objX, double * objY, double * objZ);

// Draws a sphere centred at the origin, with smooth normals (as gluSphere)
void DrawSphere(double radius, int slices, int stacks);

// Draws a cylinder along +z from z = 0 to z = height, without end caps (as gluCylinder)
void DrawCylinder(double baseRadius, double topRadius, double height, int slices, int stacks);

// Draws a disk in the z = 0 plane facing +z (as gluDisk)
void DrawDisk(double innerRadius, double outerRadius, int slices, int loops);

// Returns a description of an OpenGL error code (as gluErrorString)
const char * GLErrorString(GLenum errorCode);

#endif
