//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#include "glutils.hpp"

#include <cmath>

static const double PI = 3.14159265358979323846;

// In the drawing functions below, the vertex pairs in each quad strip are ordered so that faces
// wind anticlockwise when seen from outside, as with GLU.

//======================================================================================================================
// SetPerspective
//======================================================================================================================
void SetPerspective(double fovYDegrees, double aspect, double zNear, double zFar)
{
  double top = zNear * std::tan(fovYDegrees * PI / 360.0);
  double right = top * aspect;
  glFrustum(-right, right, -top, top, zNear, zFar);
}

//======================================================================================================================
// LookAt
//======================================================================================================================
void LookAt(double eyeX, double eyeY, double eyeZ,
            double centreX, double centreY, double centreZ,
            double upX, double upY, double upZ)
{
  // Forward
  double f[3] = {centreX - eyeX, centreY - eyeY, centreZ - eyeZ};
  double fLen = std::sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
  if (fLen > 0.0)
  {
    f[0] /= fLen; f[1] /= fLen; f[2] /= fLen;
  }
  // Side = forward x up
  double s[3] = {f[1] * upZ - f[2] * upY, f[2] * upX - f[0] * upZ, f[0] * upY - f[1] * upX};
  double sLen = std::sqrt(s[0] * s[0] + s[1] * s[1] + s[2] * s[2]);
  if (sLen > 0.0)
  {
    s[0] /= sLen; s[1] /= sLen; s[2] /= sLen;
  }
  // Recomputed up = side x forward
  double u[3] = {s[1] * f[2] - s[2] * f[1], s[2] * f[0] - s[0] * f[2], s[0] * f[1] - s[1] * f[0]};

  // Column major
  double m[16] = {
    s[0], u[0], -f[0], 0.0,
    s[1], u[1], -f[1], 0.0,
    s[2], u[2], -f[2], 0.0,
    0.0,  0.0,  0.0,   1.0};
  glMultMatrixd(m);
  glTranslated(-eyeX, -eyeY, -eyeZ);
}

//======================================================================================================================
// Matrix helpers for UnProject. Matrices are column major, as OpenGL uses.
//======================================================================================================================
static void MultMatrices(const double a[16], const double b[16], double out[16])
{
  for (int col = 0 ; col < 4 ; ++col)
  {
    for (int row = 0 ; row < 4 ; ++row)
    {
      double sum = 0.0;
      for (int k = 0 ; k < 4 ; ++k)
        sum += a[k * 4 + row] * b[col * 4 + k];
      out[col * 4 + row] = sum;
    }
  }
}

// Gauss-Jordan elimination with partial pivoting
static bool InvertMatrix(const double m[16], double out[16])
{
  double a[4][8];
  for (int row = 0 ; row < 4 ; ++row)
  {
    for (int col = 0 ; col < 4 ; ++col)
    {
      a[row][col] = m[col * 4 + row];
      a[row][col + 4] = (row == col) ? 1.0 : 0.0;
    }
  }
  for (int col = 0 ; col < 4 ; ++col)
  {
    int pivot = col;
    for (int row = col + 1 ; row < 4 ; ++row)
    {
      if (std::fabs(a[row][col]) > std::fabs(a[pivot][col]))
        pivot = row;
    }
    if (a[pivot][col] == 0.0)
      return false;
    if (pivot != col)
    {
      for (int k = 0 ; k < 8 ; ++k)
      {
        double tmp = a[col][k]; a[col][k] = a[pivot][k]; a[pivot][k] = tmp;
      }
    }
    double inv = 1.0 / a[col][col];
    for (int k = 0 ; k < 8 ; ++k)
      a[col][k] *= inv;
    for (int row = 0 ; row < 4 ; ++row)
    {
      if (row == col)
        continue;
      double factor = a[row][col];
      for (int k = 0 ; k < 8 ; ++k)
        a[row][k] -= factor * a[col][k];
    }
  }
  for (int row = 0 ; row < 4 ; ++row)
    for (int col = 0 ; col < 4 ; ++col)
      out[col * 4 + row] = a[row][col + 4];
  return true;
}

//======================================================================================================================
// UnProject
//======================================================================================================================
bool UnProject(double winX, double winY, double winZ,
               const double modelView[16], const double projection[16], const GLint viewport[4],
               double * objX, double * objY, double * objZ)
{
  double projModelView[16], inverse[16];
  MultMatrices(projection, modelView, projModelView);
  if (!InvertMatrix(projModelView, inverse))
    return false;

  // Window to normalised device coordinates
  double in[4] = {
    (winX - viewport[0]) / viewport[2] * 2.0 - 1.0,
    (winY - viewport[1]) / viewport[3] * 2.0 - 1.0,
    winZ * 2.0 - 1.0,
    1.0};

  double out[4];
  for (int row = 0 ; row < 4 ; ++row)
  {
    out[row] = 0.0;
    for (int k = 0 ; k < 4 ; ++k)
      out[row] += inverse[k * 4 + row] * in[k];
  }
  if (out[3] == 0.0)
    return false;

  *objX = out[0] / out[3];
  *objY = out[1] / out[3];
  *objZ = out[2] / out[3];
  return true;
}

//======================================================================================================================
// DrawSphere
//======================================================================================================================
void DrawSphere(double radius, int slices, int stacks)
{
  for (int i = 0 ; i < stacks ; ++i)
  {
    double phi0 = PI * i / stacks;
    double phi1 = PI * (i + 1) / stacks;
    glBegin(GL_QUAD_STRIP);
    for (int j = 0 ; j <= slices ; ++j)
    {
      double theta = 2.0 * PI * j / slices;
      double ct = std::cos(theta), st = std::sin(theta);
      double n1[3] = {ct * std::sin(phi1), st * std::sin(phi1), std::cos(phi1)};
      double n0[3] = {ct * std::sin(phi0), st * std::sin(phi0), std::cos(phi0)};
      glNormal3dv(n0);
      glVertex3d(radius * n0[0], radius * n0[1], radius * n0[2]);
      glNormal3dv(n1);
      glVertex3d(radius * n1[0], radius * n1[1], radius * n1[2]);
    }
    glEnd();
  }
}

//======================================================================================================================
// DrawCylinder
//======================================================================================================================
void DrawCylinder(double baseRadius, double topRadius, double height, int slices, int stacks)
{
  // Normal tilts when the radius changes along the length
  double slope = (baseRadius - topRadius) / height;
  double nz = slope / std::sqrt(1.0 + slope * slope);
  double nxy = 1.0 / std::sqrt(1.0 + slope * slope);
  for (int i = 0 ; i < stacks ; ++i)
  {
    double z0 = height * i / stacks;
    double z1 = height * (i + 1) / stacks;
    double r0 = baseRadius + (topRadius - baseRadius) * i / stacks;
    double r1 = baseRadius + (topRadius - baseRadius) * (i + 1) / stacks;
    glBegin(GL_QUAD_STRIP);
    for (int j = 0 ; j <= slices ; ++j)
    {
      double theta = 2.0 * PI * j / slices;
      double ct = std::cos(theta), st = std::sin(theta);
      glNormal3d(ct * nxy, st * nxy, nz);
      glVertex3d(r1 * ct, r1 * st, z1);
      glVertex3d(r0 * ct, r0 * st, z0);
    }
    glEnd();
  }
}

//======================================================================================================================
// DrawDisk
//======================================================================================================================
void DrawDisk(double innerRadius, double outerRadius, int slices, int loops)
{
  glNormal3d(0.0, 0.0, 1.0);
  for (int i = 0 ; i < loops ; ++i)
  {
    double r0 = innerRadius + (outerRadius - innerRadius) * i / loops;
    double r1 = innerRadius + (outerRadius - innerRadius) * (i + 1) / loops;
    glBegin(GL_QUAD_STRIP);
    for (int j = 0 ; j <= slices ; ++j)
    {
      double theta = 2.0 * PI * j / slices;
      double ct = std::cos(theta), st = std::sin(theta);
      glVertex3d(r0 * ct, r0 * st, 0.0);
      glVertex3d(r1 * ct, r1 * st, 0.0);
    }
    glEnd();
  }
}

//======================================================================================================================
// GLErrorString
//======================================================================================================================
const char * GLErrorString(GLenum errorCode)
{
  switch (errorCode)
  {
  case GL_NO_ERROR: return "no error";
  case GL_INVALID_ENUM: return "invalid enumerant";
  case GL_INVALID_VALUE: return "invalid value";
  case GL_INVALID_OPERATION: return "invalid operation";
  case GL_STACK_OVERFLOW: return "stack overflow";
  case GL_STACK_UNDERFLOW: return "stack underflow";
  case GL_OUT_OF_MEMORY: return "out of memory";
  default: return "unknown error";
  }
}
