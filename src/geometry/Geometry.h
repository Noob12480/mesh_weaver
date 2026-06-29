#pragma once
#include"HalfEdgeMesh.h"
#include"core/MathTypes.h"
#include<vector>

struct AABB{
    Vec3d min;
    Vec3d max;
};

class GeometryCore{
public:
    static Vec3d faceNormal(const HalfEdgeMesh &mesh, int faceId);
    static double faceArea(const HalfEdgeMesh &mesh, int faceId);
    static double surfaceArea(const HalfEdgeMesh &mesh);
    static AABB computeAABB(const HalfEdgeMesh &mesh);
    static Vec3d vertexNormal(const HalfEdgeMesh &mesh, int vertId);
    static std::vector<Vec3d> computeVertexNormals(const HalfEdgeMesh &mesh);
    static int eulerCharacteristic(const HalfEdgeMesh &mesh);
};