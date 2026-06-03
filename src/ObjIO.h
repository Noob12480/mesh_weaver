#pragma once

#include <string>
#include "Mesh.h"

class ObjIO
{
public:
    static bool load(const std::string& filename, Mesh& mesh);
    static bool save(const std::string& filename, const Mesh& mesh);
};