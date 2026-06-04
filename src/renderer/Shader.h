#pragma once

#include "core/MathTypes.h"

struct VertexInput {
    Vec3d position;
    Vec3d normal;
    Vec3d color;
};

struct VertexOutput {
    Vec4d clipPosition;
    Vec3d worldPosition;
    Vec3d normal;
    Vec3d color;
};

class Shader {
public:
    virtual ~Shader() = default;

    virtual VertexOutput vertex(const VertexInput& input) = 0;
    virtual Vec3d fragment(const VertexOutput& input) = 0;
};