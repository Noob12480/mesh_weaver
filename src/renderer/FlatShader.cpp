#include"Shader.h"

FlatShader::FlatShader(const Mat4d& MVP, const Mat4d& Model, const Vec3d& baseColor, const Vec3d& lightPos, const Vec3d& cameraPos)
: MVP(MVP),Model(Model),baseColor(baseColor),lightPos(lightPos),cameraPos(cameraPos){
    Mat3d M3 = Model.block<3, 3>(0, 0);
    normalMatrix = M3.inverse().transpose();
}
VertexOutput FlatShader::vertex(const VertexInput& input) const{
    VertexOutput o;
    Vec4d local(input.position.x(),input.position.y(),input.position.z(),1.0);

    o.clipPosition = MVP*local;
    Vec4d worldPos=Model*local;
    o.worldPosition=Vec3d(worldPos.x(),worldPos.y(),worldPos.z());
    o.normal=(normalMatrix*input.faceNormal).normalized();
    o.color=baseColor;
    return o;
}

Vec3d FlatShader::fragment(const VertexOutput& input) const{
    return BlinnPhong::shade(baseColor,input.worldPosition,input.normal,lightPos,cameraPos);
}