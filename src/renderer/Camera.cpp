#include"Camera.h"
#include<math.h>
constexpr double PI = 3.14159265358979323846;
Camera::Camera(){

}

Mat4d Camera::viewMatrix() const{
    Mat4d T;
    T<< 1, 0, 0, -position.x(),
        0, 1, 0, -position.y(),
        0, 0, 1, -position.z(),
        0, 0, 0, 1;
    
    Vec3d g=(target-position).normalized();
    Vec3d d=g.cross(up).normalized();
    Vec3d t=d.cross(g).normalized();
    Mat4d R;
    R<< d.x(), d.y(), d.z(), 0,
        t.x(), t.y(), t.z(), 0,
        -g.x(), -g.y(), -g.z(), 0,
        0, 0, 0, 1;

    return R*T;
}

Mat4d Camera::projectionMatrix() const{
    Mat4d P;

    double n=-zNear;
    double f=-zFar;

    double t=zNear*tan(fov*PI/360);
    double b=-t;
    double r=t*aspectRatio;
    double l=-r;

    P<< n, 0, 0, 0,
        0, n, 0, 0,
        0, 0, n+f, -n*f,
        0, 0, 1, 0;

    Mat4d O;
    O<< 2/(r-l), 0, 0, -(r+l)/(r-l),
        0, 2/(t-b), 0, -(t+b)/(t-b),
        0, 0, 2/(n-f), -(n+f)/(n-f),
        0, 0, 0, 1;

    return O*P;
}

void Camera::setPosition(const Vec3d& pos){
    this->position=pos;
}

void Camera::setTarget(const Vec3d& target){
    this->target=target;
}

void Camera::setUp(const Vec3d& up){
    this->up=up;
}

void Camera::setPerspective(double fov,double aspect,double near,double far){
    this->fov=fov;
    this->aspectRatio=aspect;
    this->zNear=near;
    this->zFar=far;
}

const Vec3d& Camera::getPosition() const{
    return position;
}

const Vec3d& Camera::getTarget() const{
    return target;
}

const Vec3d& Camera::getUp() const{
    return up;
}