#pragma once

#include "core/MathTypes.h"

class Camera {
public:
    Camera();

    Mat4d viewMatrix() const;
    Mat4d projectionMatrix() const;
    void setPosition(const Vec3d& pos);
    void setTarget(const Vec3d& target);
    void setUp(const Vec3d& up);
    void setPerspective(double fov,double aspect,double near,double far);

    const Vec3d& getPosition() const;
    const Vec3d& getTarget() const;
    const Vec3d& getUp() const;

private:
    Vec3d position;
    Vec3d target;
    Vec3d up;

    double fov;
    double aspectRatio;
    double zNear;
    double zFar;
};