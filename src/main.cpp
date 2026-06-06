#include<iostream>
#include<windows.h>
#include<Eigen/Core>
#include<Eigen/Geometry>
#include"ObjIO.h"
#include"string"
#include"geometry/HalfEdgeMesh.h"
#include"renderer/Rasterizer.h"
#include"renderer/Camera.h"
#include"window/Win32Window.h"
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
void updateCamera(Camera& camera, const Vec3d& modelCenter, double radius, double& yaw, double& pitch, double dt){
    const Vec3d worldUp(0, 1, 0);

    double moveSpeed = (std::max)(1.0, radius * 2.5);
    double mouseSpeed = 0.004;

    static bool firstMouse = true;
    static POINT lastMouse;

    POINT curMouse;
    GetCursorPos(&curMouse);

    if (firstMouse) {
        lastMouse = curMouse;
        firstMouse = false;
    }

    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
        double dx = double(curMouse.x - lastMouse.x);
        double dy = double(curMouse.y - lastMouse.y);

        yaw   += dx * mouseSpeed;
        pitch -= dy * mouseSpeed;

        double maxPitch = 1.55;
        pitch = std::clamp(pitch, -maxPitch, maxPitch);
    }

    lastMouse = curMouse;

    Vec3d forward;
    forward.x() = std::cos(pitch) * std::sin(yaw);
    forward.y() = std::sin(pitch);
    forward.z() = -std::cos(pitch) * std::cos(yaw);
    forward.normalize();

    Vec3d right = forward.cross(worldUp).normalized();

    Vec3d pos = camera.getPosition();
    double step = moveSpeed * dt;

    if (GetAsyncKeyState('W') & 0x8000) pos += forward * step;
    if (GetAsyncKeyState('S') & 0x8000) pos -= forward * step;
    if (GetAsyncKeyState('A') & 0x8000) pos -= right * step;
    if (GetAsyncKeyState('D') & 0x8000) pos += right * step;

    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) pos -= worldUp * step;
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) pos += worldUp * step;

    camera.setPosition(pos);
    camera.setTarget(pos + forward);
    camera.setUp(worldUp);
}
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    //光栅化渲染器
    FrameBuffer buffer(512, 512);
    Rasterizer rasterizer(buffer);
    //相机
    Camera camera;
    camera.setPosition(Vec3d(0, 0, 5));
    camera.setTarget(Vec3d(0, 0, 0));
    camera.setUp(Vec3d(0, 1, 0));
    camera.setPerspective(90, 1.0, 0.001, 100.0);
    
    Win32Window window(512, 512, L"Mesh Lab");
    //模型
    std::string filename="assets/FinalBaseMesh.obj";
    //std::string filename="assets/coca-cola.obj";
    //std::string filename="assets/Low-Poly_Models.obj";
    ObjIO io;
    Mesh mesh;
    io.load(filename,mesh);
    HalfEdgeMesh halfEdgeMesh;
    halfEdgeMesh.buildFromMesh(mesh);
    std::cout<<"网格合法性验证:\n"<<halfEdgeMesh.validate()<<'\n';

    Vec3d modelCenter(0, 0, 0);
    double radius = 5.0;
    double yaw = 0.0;
    double pitch = 0.0;

    //return 0;
    while (!window.shouldClose()) {
        window.pollEvents();

        double dt = 1.0 / 60.0;
        updateCamera(camera, modelCenter, radius, yaw, pitch, dt);

        Mat4d Model=Mat4d::Identity();
        Mat4d MVP = Model * camera.projectionMatrix() * camera.viewMatrix();

        rasterizer.resetStats();
        
        buffer.clearColor(Vec3d(0, 0, 0));
        buffer.clearDepth(1.0);

        //FlatShader shader(MVP,Vec3d(0,1,0),Vec3d(0,1,1));
        PhongShader shader(MVP,Model,Vec3d(1.0,1.0,1.0), Vec3d(0,0,5), camera.getPosition());
        rasterizer.drawMesh(halfEdgeMesh, shader);
        //rasterizer.drawMesh(halfEdgeMesh, VP, Vec3d(0,1,0));
        window.present(buffer, {camera.getPosition(),rasterizer.getStats().totalTriangles,rasterizer.getStats().renderedTriangles});
    }

    return 0;
}