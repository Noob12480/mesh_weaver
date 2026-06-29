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
#include"geometry/Geometry.h"
#include<functional>
#include<string>
#include<cmath>

struct ShaderContext {
    Mat4d MVP;
    Mat4d Model;
    const Material& material;
    Vec3d lightDir;
    Vec3d lightPos;
    Vec3d cameraPos;
};
struct ShaderEntry {
    std::string name;
    std::function<std::unique_ptr<Shader>(const ShaderContext&)> create;
};

std::vector<ShaderEntry> shaders;
int shaderIndex = 0;

void init(){

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    shaders.push_back({
        "Flat",
        [](const ShaderContext& ctx) {
            return std::make_unique<FlatShader>(
                ctx.MVP,
                ctx.Model,
                ctx.material,
                ctx.lightPos,
                ctx.cameraPos
            );
        }
    });
    shaders.push_back({
        "Phong",
        [](const ShaderContext& ctx) {
            return std::make_unique<PhongShader>(
                ctx.MVP,
                ctx.Model,
                ctx.material,
                ctx.lightPos,
                ctx.cameraPos
            );
        }
    });
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
void updateShader() {
    static bool lastR = false;
    bool curR = (GetAsyncKeyState('R') & 0x8000) != 0;
    if (curR && !lastR) {
        shaderIndex = (shaderIndex + 1) % shaders.size();
    }
    lastR = curR;
}
void updateCullMode(Rasterizer& rasterizer) {
    static bool lastF = false;
    bool curF = (GetAsyncKeyState('F') & 0x8000) != 0;
    if (curF && !lastF) {
        CullMode mode = rasterizer.getCullMode();

        if (mode == CullMode::None) {
            rasterizer.setCullMode(CullMode::Back);
            //std::cout<<"111"<<'\n';
        }
        else if (mode == CullMode::Back) {
            rasterizer.setCullMode(CullMode::Front);
        }
        else {
            rasterizer.setCullMode(CullMode::None);
        }
    }
    lastF = curF;
}
std::string cullModeName(CullMode mode) {
    switch (mode) {
    case CullMode::None: return "None";
    case CullMode::Back: return "Back";
    case CullMode::Front: return "Front";
    default: return "Unknown";
    }
}
int main(int argc, char** argv) {

    init();

    //test();
    //模型
    std::string filename="assets/models/FinalBaseMesh.obj";
    //std::string filename="assets/coca-cola.obj";
    //std::string filename="assets/Low-Poly_Models.obj";
    if (argc >= 2) {
        filename = argv[1];
    }
    ObjIO io;
    Mesh mesh;
    if (!ObjIO::load(filename, mesh)) {
        std::cerr<<"无效的路径: "<<filename<<'\n';
        return -1;
    }
    HalfEdgeMesh halfEdgeMesh;
    halfEdgeMesh.buildFromMesh(mesh);
    std::cout<<"网格合法性验证:\n"<<halfEdgeMesh.validate()<<'\n';
    
    //纹理
    Texture texture;
    //texture.loadPPM("assets/textures/coca-cola-zero.ppm");
    //材质
    Material material;
    material.setBaseColor(Vec3d(1.0,1.0,1.0));
    //material.setTexture(&texture);

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
        //PhongShader shader();
        //按R切换着色模型
        updateShader();

        //按F切换背面剔除模式
        updateCullMode(rasterizer);
        ShaderContext ctx={MVP,Model,material,Vec3d(0,0,1),Vec3d(0,0,5), camera.getPosition()};

        std::string shaderName=shaders[shaderIndex].name;
        auto shader = shaders[shaderIndex].create(ctx);

        rasterizer.drawMesh(halfEdgeMesh, *shader);
        //rasterizer.drawMesh(halfEdgeMesh, VP, Vec3d(0,1,0));
        window.present(buffer, {camera.getPosition(),
            rasterizer.getStats().totalTriangles,rasterizer.getStats().renderedTriangles,
            shaderName, cullModeName(rasterizer.getCullMode())});
    }

    return 0;
}