#pragma once

#include "core/MathTypes.h"

struct VertexInput {
    Vec3d position=Vec3d(0, 0, 0);
    Vec3d faceNormal=Vec3d(0, 0, 1);
    Vec3d vertNormal=Vec3d(0, 0, 1);
    Vec3d color=Vec3d(1, 1, 1);
};

struct VertexOutput {
    Vec4d clipPosition;
    Vec3d worldPosition;
    Vec3d normal;
    Vec3d color;
};

struct BlinnPhong {
    static constexpr double ka = 0.2;
    static constexpr double kd = 0.75;
    static constexpr double ks = 0.55;
    static constexpr double shininess = 32.0;

    static Vec3d lightColor() {
        return Vec3d(1.0, 1.0, 1.0);
    }

    static Vec3d shade(const Vec3d& baseColor,const Vec3d& worldPosition,const Vec3d& normal,const Vec3d& lightPos,const Vec3d& cameraPos) {
        Vec3d n = normal.normalized();
        Vec3d l = (lightPos - worldPosition).normalized();
        Vec3d v = (cameraPos - worldPosition).normalized();
        Vec3d h = (l + v).normalized();

        double ambient  = ka;
        double diffuse  = kd * (std::max)(0.0, n.dot(l));
        double specular = ks * std::pow((std::max)(0.0, n.dot(h)), shininess);

        return baseColor * (ambient + diffuse) + lightColor() * specular;
    }
};

class Shader {
public:
    virtual ~Shader() = default;

    virtual VertexOutput vertex(const VertexInput& input) const=0;
    virtual Vec3d fragment(const VertexOutput& input) const=0;
    virtual bool needVertNormal() const {return false;}
};

class FlatShader : public Shader {
public:
    FlatShader(const Mat4d& MVP,const Mat4d& Model,const Vec3d& baseColor,const Vec3d& lightPos,const Vec3d& cameraPos);
    VertexOutput vertex(const VertexInput& input) const;
    Vec3d fragment(const VertexOutput& input) const;

private:
    Mat4d MVP;
    Mat4d Model;
    Mat3d normalMatrix;

    Vec3d baseColor;
    Vec3d lightPos;
    Vec3d cameraPos;
};

class PhongShader : public Shader {
public:
    PhongShader(const Mat4d& MVP, const Mat4d& Model, const Vec3d& baseColor, const Vec3d& lightPos, const Vec3d& cameraPos);
    VertexOutput vertex(const VertexInput& input) const;
    Vec3d fragment(const VertexOutput& input) const;
    
    bool needVertNormal() const {return true;}

private:
    Mat4d MVP;
    Mat4d Model;
    Mat3d normalMatrix;

    Vec3d baseColor;
    Vec3d lightPos;
    Vec3d cameraPos;
};