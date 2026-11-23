//======================================================================================================================
// Copyright (C) 2004 Danny Chapman
//               https://www.rowlhouse.co.uk
//======================================================================================================================
#include "meshobject.hpp"
#include "resources.hpp"

#include <pugixml.hpp>
#include "debugconfig.hpp"

using namespace JigLib;

//======================================================================================================================
// SetProperties
//======================================================================================================================
void tMeshObject::SetProperties(tScalar elasticity,
                                tScalar staticFriction,
                                tScalar dynamicFriction)
{
  mCollisionSkin.SetMaterialProperties(0, tMaterialProperties(elasticity, staticFriction, dynamicFriction));
}

//======================================================================================================================
// tMeshObject
//======================================================================================================================
tMeshObject::tMeshObject(const std::vector<tVector3> &vertices,
                         const std::vector<tTriangleVertexIndices> &triangleVertexIndices,
                         int maxTrianglesPerCell, tScalar minCellSize)
{
  TRACE_METHOD_ONLY(ONCE_2);

  tTriangleMesh mesh;

  // Create the mesh
  if (!vertices.empty() && !triangleVertexIndices.empty())
    mesh.CreateMesh(&vertices[0], vertices.size(),
    &triangleVertexIndices[0], triangleVertexIndices.size(),
    maxTrianglesPerCell, minCellSize);
  else
    TRACE("Unable to create mesh from vertex list\n");

  mCollisionSkin.AddPrimitive(mesh, tMaterialTable::UNSET);

  SetProperties(0.6f, 0.4f, 0.2f);

  mRenderPosition.SetTo(0.0f);
  mRenderOrientation.SetToIdentity();

  mDisplayListNum = 0;
}


//======================================================================================================================
// tMeshObject
//======================================================================================================================
tMeshObject::tMeshObject(const char* meshXMLFile, tScalar meshScale,
                         int maxTrianglesPerCell, tScalar minCellSize)
{
  TRACE_METHOD_ONLY(ONCE_2);

  // Load the mesh
  std::vector<tVector3> vertices;
  std::vector<tTriangleVertexIndices> triangleVertexIndices;
  LoadMesh(meshXMLFile, vertices, triangleVertexIndices);

  // scale it
  for (unsigned i = 0 ; i < vertices.size() ; ++i)
    vertices[i] *= meshScale;

  tTriangleMesh mesh;

  // Create the mesh
  if (!vertices.empty() && !triangleVertexIndices.empty())
    mesh.CreateMesh(&vertices[0], vertices.size(),
                         &triangleVertexIndices[0], triangleVertexIndices.size(),
                         maxTrianglesPerCell, minCellSize);
  else
    TRACE("Unable to create mesh from %s\n", meshXMLFile);

  mCollisionSkin.AddPrimitive(mesh, tMaterialTable::UNSET);

  SetProperties(0.6f, 0.4f, 0.2f);

  mRenderPosition.SetTo(0.0f);
  mRenderOrientation.SetToIdentity();

  mDisplayListNum = 0;
}

//======================================================================================================================
// ~tMeshObject
//======================================================================================================================
tMeshObject::~tMeshObject()
{
  TRACE_METHOD_ONLY(ONCE_2);
}


//======================================================================================================================
// GetRenderBoundingMesh
//======================================================================================================================
const tSphere & tMeshObject::GetRenderBoundingSphere() const
{
  // Ugly...
  static tSphere sphere;
  sphere.SetPos(mCollisionSkin.GetWorldBoundingBox().GetCentre());
  sphere.SetRadius(mCollisionSkin.GetWorldBoundingBox().GetRadiusAboutCentre());
  return sphere;
}

//======================================================================================================================
// GetRenderPosition
//======================================================================================================================
const tVector3 & tMeshObject::GetRenderPosition() const
{
  return mRenderPosition;
}

//======================================================================================================================
// GetRenderOrientation
//======================================================================================================================
const tMatrix33 & tMeshObject::GetRenderOrientation() const
{
  return mRenderOrientation;
}

//======================================================================================================================
// Render
//======================================================================================================================
void tMeshObject::Render(tRenderObject::tRenderType renderType)
{
  TRACE_METHOD_ONLY(FRAME_2);

  if (mDisplayListNum == 0)
  {
    mDisplayListNum = glGenLists(1);
    glNewList(mDisplayListNum, GL_COMPILE);
    GLCOLOR4(RangedRandom(0.8f, 0.9f),
             RangedRandom(0.8f, 0.9f),
             RangedRandom(0.8f, 0.9f),
             1.0f);

    const tTriangleMesh& mesh = mCollisionSkin.GetPrimitiveLocal(0)->GetTriangleMesh();

    glBegin(GL_TRIANGLES);

    unsigned numTriangles = mesh.GetNumTriangles();
    for (unsigned iTriangle = 0 ; iTriangle < numTriangles ; ++iTriangle)
    {
#if 0
      GLCOLOR4(RangedRandom(0.2f, 1.0f),
               RangedRandom(0.2f, 1.0f),
               RangedRandom(0.2f, 1.0f),
               0.5f);
#endif
      const tIndexedTriangle& triangle = mesh.GetTriangle(iTriangle);
      unsigned i0, i1, i2;
      triangle.GetVertexIndices(i0, i1, i2);
      const tVector3& v0 = mesh.GetVertex(i0);
      const tVector3& v1 = mesh.GetVertex(i1);
      const tVector3& v2 = mesh.GetVertex(i2);
      const tVector3& N = triangle.GetPlane().GetN();

      GLNORMAL3V(N.GetData());
      GLVERTEX3V(v0.GetData());
      GLVERTEX3V(v1.GetData());
      GLVERTEX3V(v2.GetData());
    }
    glEnd();

    glEndList();
  }

  if (tDebugConfig::mRenderCollisionBoundingBoxes)
    RenderWorldBoundingBox();

  ApplyTransformation(mRenderPosition, mRenderOrientation);
  glCallList(mDisplayListNum);
}

//======================================================================================================================
// AddToTriangles
//======================================================================================================================
void tMeshObject::AddToTriangles(std::vector<tVector3>& triangleVertices,
                                 std::vector<tTriangleVertexIndices>& triangleVertexIndices,
                                 const std::vector<tVector3>& vertices,
                                 const std::vector<tTriangleVertexIndices>& faces)
{
  unsigned offset = triangleVertices.size();
  unsigned i;
  for (i = 0 ; i < vertices.size() ; ++i)
    triangleVertices.push_back(vertices[i]);
  for (i = 0 ; i < faces.size() ; ++i)
  {
    if (faces[i].i0 >= vertices.size() ||faces[i].i1 >= vertices.size() ||faces[i].i2 >= vertices.size())
    {
      TRACE("face indices out of range!\n");
    }
    else
    {
      triangleVertexIndices.push_back(faces[i]);
      triangleVertexIndices.back().i0 += offset;
      triangleVertexIndices.back().i1 += offset;
      triangleVertexIndices.back().i2 += offset;
    }
  }
}

//======================================================================================================================
// LoadMesh
//
// Reads the XML format written by the Ogre3D exporter for the DeleD editor:
// <mesh><submeshes><submesh>
//   <faces count="n"><face v1="" v2="" v3=""/>...</faces>
//   <geometry vertexcount="n"><vertexbuffer><vertex><position x="" y="" z=""/>...
//======================================================================================================================
void tMeshObject::LoadMesh(const char* meshXMLFile,
                           std::vector<tVector3>& vertices,
                           std::vector<tTriangleVertexIndices>& triangleVertexIndices)
{
  vertices.clear();
  triangleVertexIndices.clear();

  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file(GetResourcePath(meshXMLFile).c_str());
  if (!result)
  {
    TRACE("Unable to load %s: %s\n", meshXMLFile, result.description());
    return;
  }

  for (pugi::xml_node submesh : doc.child("mesh").child("submeshes").children("submesh"))
  {
    std::vector<tTriangleVertexIndices> faces;
    for (pugi::xml_node face : submesh.child("faces").children("face"))
    {
      // Need to swap two vertices to convert the winding
      faces.push_back(tTriangleVertexIndices(face.attribute("v2").as_int(),
                                             face.attribute("v1").as_int(),
                                             face.attribute("v3").as_int()));
    }

    std::vector<tVector3> submeshVertices;
    for (pugi::xml_node vertex : submesh.child("geometry").child("vertexbuffer").children("vertex"))
    {
      // DeleD coordinates are left-handed, and y is up... So the mapping onto my coords is:
      // x -> -y
      // y -> z
      // z -> x
      pugi::xml_node position = vertex.child("position");
      tScalar x = position.attribute("x").as_float();
      tScalar y = position.attribute("y").as_float();
      tScalar z = position.attribute("z").as_float();
      submeshVertices.push_back(tVector3(z, -x, y));
    }

    TRACE("Read %d faces and %d vertices\n", (int) faces.size(), (int) submeshVertices.size());
    AddToTriangles(vertices, triangleVertexIndices, submeshVertices, faces);
  }
}

