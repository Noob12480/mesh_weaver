#pragma once

#include <string>
#include "geometry/Mesh.h"
#include<geometry/HalfEdgeMesh.h>
class ObjIO
{
public:
    static bool load(const std::string& filename, Mesh& mesh);
    static bool save(const std::string& filename, const Mesh& mesh);
    static bool save(const std::string& filename, const HalfEdgeMesh& mesh);
};