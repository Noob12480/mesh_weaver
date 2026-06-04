#include<iostream>
#include<windows.h>
#include<Eigen/Core>
#include<Eigen/Geometry>
#include"ObjIO.h"
#include"string"
#include"HalfEdgeMesh.h"
#include"renderer/Rasterizer.h"
#include"renderer/Camera.h"
void testHalfEdgeMash(){
    std::string filename="assets/coca-cola.obj";
    ObjIO io;
    Mesh mesh;
    io.load(filename,mesh);
    
    // int target = 64;
    // int count = 0;

    // for (int fi = 0; fi < mesh.faces.size(); ++fi) {
    //     for (auto idx : mesh.faces[fi].indices) {
    //         if (idx.v == target) {
    //             std::cout << "internal vertex " << target
    //                     << " used in face " << fi << '\n';
    //             count++;
    //         }
    //     }
    // }

    // std::cout << "used count = " << count << '\n';
    
    HalfEdgeMesh halfEdgeMesh;
    halfEdgeMesh.buildFromMesh(mesh);
    std::cout<<"网格合法性验证:\n"<<halfEdgeMesh.validate()<<'\n';

    std::vector<int> v1=halfEdgeMesh.faceVertices(0);
    std::cout<<"面0的顶点:"<<'\n';
    for(auto v:v1){
        std::cout<<v<<' ';
    }
    std::cout<<'\n';

    std::vector<int> e1=halfEdgeMesh.faceEdges(0);
    std::cout<<"面0的边:"<<'\n';
    for(auto e:e1){
        std::cout<<e<<' ';
    }
    std::cout<<'\n';

    int prevVert=halfEdgeMesh.edgeOrigin(0);
    int targetVert=halfEdgeMesh.edgeTarget(0);
    std::cout<<"边0: "<<prevVert<<' '<<targetVert<<'\n';

    std::vector<int> v2=halfEdgeMesh.vertexNeighbors(290);
    std::cout<<"顶点290的邻接顶点:"<<'\n';
    for(auto v:v2){
        std::cout<<v<<' ';
    }
    std::cout<<'\n'; 

    std::vector<int> e2=halfEdgeMesh.vertexEdges(290);
    std::cout<<"顶点290的邻接边:"<<'\n';
    for(auto e:e2){
        std::cout<<e<<' ';
    }
    std::cout<<'\n'; 

    std::vector<int> f1=halfEdgeMesh.vertexFaces(290);
    std::cout<<"顶点290的邻接面:"<<'\n';
    for(auto f:f1){
        std::cout<<f<<' ';
    }
    std::cout<<'\n'; 
}
void testRenderer(){
    FrameBuffer buffer(512, 512);
    buffer.clearColor(Vec3d(0, 0, 0));
    buffer.clearDepth(1.0);

    Rasterizer rasterizer(buffer);

    Camera camera;
    camera.setPosition(Vec3d(0, 0, 5));
    camera.setTarget(Vec3d(0, 0, 0));
    camera.setUp(Vec3d(0, 1, 0));
    camera.setPerspective(90, 1.0, 1.0, 10.0);

    Mat4d VP = camera.projectionMatrix() * camera.viewMatrix();

    Vec3d p0(-1, -1, 0);
    Vec3d p1( 1, -1, 0);
    Vec3d p2( 0,  1, 0);

    rasterizer.drawTriangle3D(p0, p1, p2, VP, Vec3d(0, 1, 0));

    buffer.savePPM("test.ppm");
}
Vec3d projectPoint(const Mat4d& M, const Vec3d& p) {
    Eigen::Vector4d hp(p.x(), p.y(), p.z(), 1.0);
    Eigen::Vector4d clip = M * hp;

    return Vec3d(
        clip.x() / clip.w(),
        clip.y() / clip.w(),
        clip.z() / clip.w()
    );
}
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    testRenderer();
    //testHalfEdgeMash();

    return 0;
}