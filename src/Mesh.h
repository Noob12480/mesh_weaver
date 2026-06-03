#pragma once
#include<vector>
#include<Eigen/Core>
struct ObjIndex{
    int v,vt,vn;
};
struct Face{
    std::vector<ObjIndex> indices;
};
struct Mesh{
    std::vector<Eigen::Vector3d> positions;
    std::vector<Eigen::Vector2d> texcoords;
    std::vector<Eigen::Vector3d> normals;
    std::vector<Face> faces;
};