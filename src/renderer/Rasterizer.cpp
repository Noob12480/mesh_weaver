#include"Rasterizer.h"
#include"iostream"
Rasterizer::Rasterizer(FrameBuffer &buffer) : buffer(buffer){}

//叉乘
double cross(const Vec2d &v1, const Vec2d &v2){
    return v1.x()*v2.y()-v2.x()*v1.y();
}
//顶点法线
std::vector<Vec3d> computeVertNormals(const HalfEdgeMesh& mesh) {
    const auto& vertices = mesh.getVertices();
    const auto& faces = mesh.getFaces();

    std::vector<Vec3d> vertNormals(vertices.size(), Vec3d(0, 0, 0));

    auto position = [&](int vid) -> Vec3d {
        const auto& v=vertices.at(vid);
        return Vec3d(v.x, v.y, v.z);
    };

    for (int fid = 0; fid < faces.size(); ++fid) {
        std::vector<int> vi = mesh.faceVertices(fid);
        if(vi.size()<3) continue;

        Vec3d faceNormal(0, 0, 0);

        //Newell
        for (int i=0;i<vi.size();i++) {
            int j=(i+1)%vi.size();
            Vec3d p0=position(vi[i]);
            Vec3d p1=position(vi[j]);

            faceNormal.x()+=(p0.y()-p1.y())*(p0.z()+p1.z());
            faceNormal.y()+=(p0.z()-p1.z())*(p0.x()+p1.x());
            faceNormal.z()+=(p0.x()-p1.x())*(p0.y()+p1.y());
        }
        if (faceNormal.norm() < 1e-12) continue;
        faceNormal.normalize();

        for (int vid : vi) {
            vertNormals.at(vid) += faceNormal;
        }
    }
    for (auto& n : vertNormals) {
        if (n.norm() > 1e-12) {
            n.normalize();
        } else {
            n = Vec3d(0, 0, 1);
        }
    }

    return vertNormals;
}

void Rasterizer::drawTriangle2D(const Vec2d& p0,const Vec2d& p1,const Vec2d& p2,const Vec3d& color){
    //AABB
    int minX,minY,maxX,maxY;
    minX=std::min(std::floor(p0.x()),std::min(std::floor(p1.x()),std::floor(p2.x())));
    minY=std::min(std::floor(p0.y()),std::min(std::floor(p1.y()),std::floor(p2.y())));
    maxX=std::max(std::ceil(p0.x()),std::max(std::ceil(p1.x()),std::ceil(p2.x())));
    maxY=std::max(std::ceil(p0.y()),std::max(std::ceil(p1.y()),std::ceil(p2.y())));

    //收缩到窗口内
    minX=std::max(minX,0);
    minY=std::max(minY,0);
    maxX=std::min(maxX,buffer.getWidth());
    maxY=std::min(maxY,buffer.getHeight());

    for(int y=minY;y<=maxY;y++){
        for(int x=minX;x<=maxX;x++){
            //叉乘
            Vec2d p(x+0.5,y+0.5);
            
            Vec2d p0p=p0-p;
            Vec2d p1p=p1-p;
            Vec2d p2p=p2-p;
            Vec2d p0p1=p1-p0;
            Vec2d p1p2=p2-p1;
            Vec2d p2p0=p0-p2;
            
            double ap2=cross(p0p,p1p);
            double ap0=cross(p1p,p2p);
            double ap1=cross(p2p,p0p);
            if((ap0>=0&&ap1>=0&&ap2>=0)||(ap0<=0&&ap1<=0&&ap2<=0)){
                buffer.setPixel(x,y,color);
            }
        }
    }
}

void Rasterizer::drawTriangle(const Vec3d& p0,const Vec3d& p1,const Vec3d& p2,const Vec3d& color){
    //AABB
    int minX,minY,maxX,maxY;
    minX=std::min(std::floor(p0.x()),std::min(std::floor(p1.x()),std::floor(p2.x())));
    minY=std::min(std::floor(p0.y()),std::min(std::floor(p1.y()),std::floor(p2.y())));
    maxX=std::max(std::ceil(p0.x()),std::max(std::ceil(p1.x()),std::ceil(p2.x())));
    maxY=std::max(std::ceil(p0.y()),std::max(std::ceil(p1.y()),std::ceil(p2.y())));

    //收缩到窗口内
    minX=std::max(minX,0);
    minY=std::max(minY,0);
    maxX=std::min(maxX,buffer.getWidth()-1);
    maxY=std::min(maxY,buffer.getHeight()-1);

    for(int y=minY;y<=maxY;y++){
        for(int x=minX;x<=maxX;x++){
            //叉乘
            Vec2d p(x+0.5,y+0.5);
            
            Vec2d p0p(p.x()-p0.x(),p.y()-p0.y());
            Vec2d p1p(p.x()-p1.x(),p.y()-p1.y());
            Vec2d p2p(p.x()-p2.x(),p.y()-p2.y());
            Vec2d p0p1(p1.x()-p0.x(),p1.y()-p0.y());
            Vec2d p1p2(p2.x()-p1.x(),p2.y()-p1.y());
            Vec2d p2p0(p0.x()-p2.x(),p0.y()-p2.y());

            double ap2=cross(p0p,p1p);
            double ap0=cross(p1p,p2p);
            double ap1=cross(p2p,p0p);
            if((ap0>=0&&ap1>=0&&ap2>=0)||(ap0<=0&&ap1<=0&&ap2<=0)){
                //重心坐标插值
                double ap2=cross(p0p,p1p);
                double ap0=cross(p1p,p2p);
                double ap1=cross(p2p,p0p);
                double a=ap0+ap1+ap2;
                if (std::abs(a)<1e-8) continue;

                double depth=(ap0*p0.z()+ap1*p1.z()+ap2*p2.z())/a;
                
                if(buffer.depthTest(x,y,depth)){
                    buffer.setDepth(x,y,depth);
                    buffer.setPixel(x,y,color);
                    //std::cout<<"1111"<<'\n';
                }
            }
        }
    }
}

void Rasterizer::drawTriangle3D(const Vec3d& p0,const Vec3d& p1,const Vec3d& p2,const Mat4d& MVP,const Vec3d& color){
    //3d tri->2d tri
    Vec3d sp0,sp1,sp2;
    if(!transPoint(p0,MVP,sp0)||!transPoint(p1,MVP,sp1)||!transPoint(p2,MVP,sp2))return;
    //std::cout<<sp0<<' '<<sp1<<' '<<sp2<<' '<<'\n';
    drawTriangle(sp0,sp1,sp2,color);
}

void Rasterizer::drawMesh(const HalfEdgeMesh &mesh, const Mat4d &MVP, const Vec3d &color){
    for(int i=0;i<mesh.getFaces().size();i++){
        std::vector<int> vi=mesh.faceVertices(i);

        if(vi.size()<3) continue;
        int o=vi[0];
        const HEVert &v0=mesh.getVertices().at(o);
        Vec3d p0(v0.x,v0.y,v0.z);

        for(int j=1;j+1<vi.size();j++){
            int vertId1=vi[j];
            int vertId2=vi[j+1];

            const HEVert &v1=mesh.getVertices().at(vertId1);
            const HEVert &v2=mesh.getVertices().at(vertId2);
            
            Vec3d p1(v1.x,v1.y,v1.z);
            Vec3d p2(v2.x,v2.y,v2.z);

            drawTriangle3D(p0,p1,p2,MVP,color);
        }
    }
}

void Rasterizer::drawTriangle(const VertexOutput& o0,const VertexOutput& o1,const VertexOutput& o2,const Shader &shader){
    // if (crossNearPlane(o0, o1, o2)) {
    //     std::cout << "cross near plane\n";
    // }

    Vec3d p0,p1,p2;
    Vec3d ndc0,ndc1,ndc2;
    if(!clipToNDC(o0.clipPosition,ndc0)||!clipToNDC(o1.clipPosition,ndc1)||!clipToNDC(o2.clipPosition,ndc2))return;
    p0=NDCToScreen(ndc0);
    p1=NDCToScreen(ndc1);
    p2=NDCToScreen(ndc2);

    //AABB
    int minX,minY,maxX,maxY;
    minX=std::min(std::floor(p0.x()),std::min(std::floor(p1.x()),std::floor(p2.x())));
    minY=std::min(std::floor(p0.y()),std::min(std::floor(p1.y()),std::floor(p2.y())));
    maxX=std::max(std::ceil(p0.x()),std::max(std::ceil(p1.x()),std::ceil(p2.x())));
    maxY=std::max(std::ceil(p0.y()),std::max(std::ceil(p1.y()),std::ceil(p2.y())));

    //收缩到窗口内
    minX=std::max(minX,0);
    minY=std::max(minY,0);
    maxX=std::min(maxX,buffer.getWidth()-1);
    maxY=std::min(maxY,buffer.getHeight()-1);

    //预计算
    double iw0=1.0/o0.clipPosition.w();
    double iw1=1.0/o1.clipPosition.w();
    double iw2=1.0/o2.clipPosition.w();

    Vec2d p0p1(p1.x()-p0.x(),p1.y()-p0.y());
    Vec2d p1p2(p2.x()-p1.x(),p2.y()-p1.y());

    double a=cross(p0p1,p1p2);
    if(std::abs(a)<1e-8) return;

    bool frontFace=a<1e-8;
    //std::cout<<a<<'\n';

    if(cullMode==CullMode::Back&&!frontFace){
        //std::cout<<"111"<<'\n';
        return;
    }
    if(cullMode==CullMode::Front&&frontFace){
        //std::cout<<"111"<<'\n';
        return;
    }

    for(int y=minY;y<=maxY;y++){
        for(int x=minX;x<=maxX;x++){
            //叉乘
            Vec2d p(x+0.5,y+0.5);
                
            Vec2d p0p(p.x()-p0.x(),p.y()-p0.y());
            Vec2d p1p(p.x()-p1.x(),p.y()-p1.y());
            Vec2d p2p(p.x()-p2.x(),p.y()-p2.y());

            double ap2=cross(p0p,p1p);
            double ap0=cross(p1p,p2p);
            double ap1=cross(p2p,p0p);

            if((ap0>=0&&ap1>=0&&ap2>=0)||(ap0<=0&&ap1<=0&&ap2<=0)){
                //深度插值

                double depth=(ap0*p0.z()+ap1*p1.z()+ap2*p2.z())/a;

                if(buffer.depthTest(x,y,depth)){
                        //插值
                    double s0=ap0/a;
                    double s1=ap1/a;
                    double s2=ap2/a;

                    double denom=s0*iw0+s1*iw1+s2*iw2;
                    if (std::abs(denom)<1e-8) continue;

                    double a0=s0*iw0/denom;
                    double a1=s1*iw1/denom;
                    double a2=s2*iw2/denom;

                    VertexOutput o;
                    o.worldPosition=a0*o0.worldPosition+a1*o1.worldPosition+a2*o2.worldPosition;
                    o.normal=a0*o0.normal+a1*o1.normal+a2*o2.normal;
                    o.color=a0*o0.color+a1*o1.color+a2*o2.color;
                        
                    Vec3d color=shader.fragment(o);
                        
                    buffer.setDepth(x,y,depth);
                    buffer.setPixel(x,y,color);
                    //std::cout<<"1111"<<'\n';
                }
            }
        }
    }   
}

void Rasterizer::drawTriangle3D(const VertexOutput &o0,const VertexOutput &o1,const VertexOutput &o2,const Shader &shader){
    // if(o0.clipPosition.w()>0||o1.clipPosition.w()>0||o2.clipPosition.w()>0){
    //     return;
    // }
    if(rejectClip(o0,o1,o2)){
        return;
    }
    if(acceptClip(o0,o1,o2)){
        drawTriangle(o0,o1,o2,shader);
        return;
    }

    std::vector<VertexOutput> poly=clipFrustum(o0,o1,o2);
    if(poly.size()<3){
        return;
    }

    for(int i=1;i+1<poly.size();i++){
        const VertexOutput& oo0 = poly[0];
        const VertexOutput& oo1 = poly[i];
        const VertexOutput& oo2 = poly[i + 1];
        drawTriangle(o0,o1,o2,shader);
    }
}

void Rasterizer::drawMesh(const HalfEdgeMesh &mesh, const Shader &shader){
    //顶点法线
    std::vector<Vec3d> vertNormals;

    if(shader.needVertNormal()){
        vertNormals=computeVertNormals(mesh);
    }
    for(int i=0;i<mesh.getFaces().size();i++){
        std::vector<int> vi=mesh.faceVertices(i);

        if(vi.size()<3) continue;
        int o=vi[0];
        const HEVert &vert0=mesh.getVertices().at(o);
        Vec3d p0(vert0.x,vert0.y,vert0.z);

        for(int j=1;j+1<vi.size();j++){
            int vertId1=vi[j];
            int vertId2=vi[j+1];

            const HEVert &vert1=mesh.getVertices().at(vertId1);
            const HEVert &vert2=mesh.getVertices().at(vertId2);
            
            Vec3d p1(vert1.x,vert1.y,vert1.z);
            Vec3d p2(vert2.x,vert2.y,vert2.z);

            //面法线
            Vec3d fn=(p1-p0).cross(p2-p1).normalized();
            VertexInput v0={p0,fn},v1={p1,fn},v2={p2,fn};

            if(shader.needVertNormal()){
                v0.vertNormal=vertNormals[o];
                v1.vertNormal=vertNormals[vertId1];
                v2.vertNormal=vertNormals[vertId2];
            }

            VertexOutput o0,o1,o2;
            o0=shader.vertex(v0);
            o1=shader.vertex(v1);
            o2=shader.vertex(v2);

            drawTriangle3D(o0,o1,o2,shader);
        }
    }
}

bool Rasterizer::insideClipSpace(const Vec4d& clip) const {
    double w = clip.w();
    return w>1e-8
        &&clip.x()>=-w&&clip.x()<= w&&clip.y()>=-w&&clip.y()<= w
        &&clip.z()>=-w&&clip.z()<=w;
}

bool Rasterizer::clipToNDC(const Vec4d& clip, Vec3d& ndc) const {
    if (std::abs(clip.w()) < 1e-8) {
        return false;
    }
    ndc = Vec3d(
        clip.x() / clip.w(),
        clip.y() / clip.w(),
        clip.z() / clip.w()
    );
    return true;
}

Vec3d Rasterizer::NDCToScreen(const Vec3d &pos) const{
    return Vec3d((pos.x()+1.0)*0.5*(buffer.getWidth()-1), (-pos.y()+1.0)*0.5*(buffer.getHeight()-1), (-pos.z()+1.0)*0.5);
}

bool Rasterizer::transPoint(const Vec3d &p, const Mat4d &MVP, Vec3d &screenPos) const{
    Vec4d pp(p.x(),p.y(),p.z(),1);

    Vec4d clip=MVP*pp;

    // if (clip.w() >= 0) {
    //     // std::cout<<"存在距离为负数的平面: "<<clip.w()<<'\n';
    //     return false;
    // }

    Vec3d ndc;
    if(!clipToNDC(clip, ndc))return false;
    screenPos=NDCToScreen(ndc);
    return true;
}

//视锥裁剪
bool Rasterizer::rejectClip(const VertexOutput &a, const VertexOutput &b, const VertexOutput &c) const{
    for(int p=0;p<6;p++){
        if(side(a,p)<-1e-8&&side(b,p)<-1e-8&&side(c,p)<-1e-8){
            //std::cout<<"111"<<'\n';
            return true;
        }
    }
    return false;
}

bool Rasterizer::acceptClip(const VertexOutput &a, const VertexOutput &b, const VertexOutput &c) const{
    for(int p=0;p<6;p++){
        if(side(a,p)>-1e-8&&side(b,p)>-1e-8&&side(c,p)>-1e-8){
            //std::cout<<"111"<<'\n';
            return true;
        }
    }
    return false;
}

double Rasterizer::side(const VertexOutput &v, int plane) const{
    const Vec4d& c=v.clipPosition;

    switch(plane){
        case 0:
            return -c.x()-c.w();
        case 1:
            return c.x()-c.w();
        case 2:
            return -c.y()-c.w();
        case 3:
            return c.y()-c.w();
        case 4:
            return c.z()-c.w();
        case 5:
            return -c.w()-c.z();
        default:
            return 0.0;
    }
}

VertexOutput Rasterizer::lerpOut(const VertexOutput &a, const VertexOutput &b, double t) const{
    VertexOutput r;
    r.clipPosition=a.clipPosition+t*(b.clipPosition-a.clipPosition);
    r.worldPosition=a.worldPosition+t*(b.worldPosition-a.worldPosition);
    r.normal=a.normal+t*(b.normal-a.normal);
    r.color=a.color+t*(b.color-a.color);
    return r;
}

std::vector<VertexOutput> Rasterizer::clipPlane(const std::vector<VertexOutput> &poly, int plane) const{
    std::vector<VertexOutput> o;

    if(poly.empty()){
        return o;
    }

    int n=poly.size();

    for(int i=0;i<n;i++){
        auto &cur=poly[i];
        auto &next=poly[(i+1)%n];

        double sc=side(cur, plane);
        double sn=side(next, plane);

        bool curIn=(sc>=-1e-8);
        bool nextIn=(sn>=-1e-8);

        if(curIn&&nextIn){
            o.push_back(next);
        } else if(curIn&&!nextIn){
            double t= sc/(sc-sn);
            o.push_back(lerpOut(cur,next,t));
        } else if(!curIn&&nextIn){
            double t=sc/(sc-sn);
            o.push_back(lerpOut(cur,next,t));
            o.push_back(next);
        }
    }
    return o;
}

std::vector<VertexOutput> Rasterizer::clipFrustum(const VertexOutput &a, const VertexOutput &b, const VertexOutput &c) const{
    std::vector<VertexOutput> poly={a,b,c};

    for(int p=0;p<6;p++){
        poly=clipPlane(poly,p);
        if(poly.size()<3){
            return {};
        }
    }
    return poly;
}