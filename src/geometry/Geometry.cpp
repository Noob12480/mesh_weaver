#include"Geometry.h"
namespace{
    std::vector<Vec3d> vertexNormalSums(const HalfEdgeMesh &mesh){
        std::vector<Vec3d> sums(mesh.getVertices().size(), Vec3d(0, 0, 0));
        int vertexCount=static_cast<int>(sums.size());

        for(int i=0; i<mesh.getFaces().size(); i++){
            std::vector<int> ids=mesh.faceVertices(i);

            if(ids.size()<3){
                continue;
            }

            Vec3d fn=GeometryCore::faceNormal(mesh, i);
            double area=GeometryCore::faceArea(mesh, i);

            if(fn.norm()<1e-12||area<1e-12){
                continue;
            }

            Vec3d weighted=fn*area;

            for(auto id:ids){
                if(id>=0&&id<vertexCount){
                    sums.at(id)+=weighted;
                }
            }
        }

        return sums;
    }

    Vec3d normalizeOrZero(const Vec3d &v){
        double len=v.norm();
        if(len<1e-12){
            return Vec3d(0, 0, 0);
        }

        return v/len;
    }
}
Vec3d GeometryCore::faceNormal(const HalfEdgeMesh &mesh, int faceId){
    if(faceId<0||faceId>=mesh.getFaces().size()){
        return Vec3d(0, 0, 0);
    }

    std::vector<int> ids=mesh.faceVertices(faceId);
    const std::vector<HEVert> &vertices=mesh.getVertices();

    if(ids.size()<3){
        return Vec3d(0, 0, 0);
    }

    Vec3d normal(0, 0, 0);

    for(int i=0; i<ids.size(); i++){
        int a=ids.at(i);
        int b=ids.at((i+1)%ids.size());

        if(a<0||a>=vertices.size()||b<0||b>=vertices.size()){
            return Vec3d(0, 0, 0);
        }

        Vec3d p(vertices.at(a).x, vertices.at(a).y, vertices.at(a).z);
        Vec3d q(vertices.at(b).x, vertices.at(b).y, vertices.at(b).z);

        normal.x()+=(p.y()-q.y())*(p.z()+q.z());
        normal.y()+=(p.z()-q.z())*(p.x()+q.x());
        normal.z()+=(p.x()-q.x())*(p.y()+q.y());
    }

    double len=normal.norm();
    if(len<1e-12){
        return Vec3d(0, 0, 0);
    }

    return normal/len;
}

double GeometryCore::faceArea(const HalfEdgeMesh &mesh, int faceId){
    if(faceId<0||faceId>=mesh.getFaces().size()){
        return 0.0;
    }

    std::vector<int> ids=mesh.faceVertices(faceId);
    const std::vector<HEVert> &vertices=mesh.getVertices();

    if(ids.size()<3){
        return 0.0;
    }

    int first=ids.at(0);
    if(first<0||first>=vertices.size()){
        return 0.0;
    }

    Vec3d p0(vertices.at(first).x, vertices.at(first).y, vertices.at(first).z);
    double area=0.0;

    for(int i=1; i+1<ids.size(); i++){
        int a=ids.at(i);
        int b=ids.at(i+1);

        if(a<0||a>=vertices.size()||b<0||b>=vertices.size()){
            return 0.0;
        }

        Vec3d p1(vertices.at(a).x, vertices.at(a).y, vertices.at(a).z);
        Vec3d p2(vertices.at(b).x, vertices.at(b).y, vertices.at(b).z);

        area+=0.5*((p1-p0).cross(p2-p0)).norm();
    }

    return area;
}

double GeometryCore::surfaceArea(const HalfEdgeMesh &mesh){
    double total=0.0;

    for(int i=0; i<mesh.getFaces().size(); i++){
        total+=faceArea(mesh, i);
    }

    return total;
}

AABB GeometryCore::computeAABB(const HalfEdgeMesh &mesh){
    const std::vector<HEVert> &vertices=mesh.getVertices();

    AABB box;
    if(vertices.empty()){
        box.min=Vec3d(0, 0, 0);
        box.max=Vec3d(0, 0, 0);
        return box;
    }

    box.min=Vec3d(vertices.at(0).x, vertices.at(0).y, vertices.at(0).z);
    box.max=box.min;

    for(int i=1; i<vertices.size(); i++){
        const HEVert &v=vertices.at(i);

        if(v.x<box.min.x()){
            box.min.x()=v.x;
        }

        if(v.y<box.min.y()){
            box.min.y()=v.y;
        }

        if(v.z<box.min.z()){
            box.min.z()=v.z;
        }

        if(v.x>box.max.x()){
            box.max.x()=v.x;
        }

        if(v.y>box.max.y()){
            box.max.y()=v.y;
        }

        if(v.z>box.max.z()){
            box.max.z()=v.z;
        }
    }

    return box;
}

Vec3d GeometryCore::vertexNormal(const HalfEdgeMesh &mesh, int vertId){
    if(vertId<0||vertId>=mesh.getVertices().size()){
        return Vec3d(0, 0, 0);
    }

    std::vector<Vec3d> sums=vertexNormalSums(mesh);
    return normalizeOrZero(sums.at(vertId));
}

std::vector<Vec3d> GeometryCore::computeVertexNormals(const HalfEdgeMesh &mesh){
    std::vector<Vec3d> sums=vertexNormalSums(mesh);
    std::vector<Vec3d> normals;
    normals.reserve(sums.size());

    for(auto n:sums){
        normals.push_back(normalizeOrZero(n));
    }

    return normals;
}

int GeometryCore::eulerCharacteristic(const HalfEdgeMesh &mesh){
    ValidationReport report=mesh.validateReport();
    return report.vertexCount-report.undirectedEdgeCount+report.faceCount;
}


namespace{
void laplacianStep(HalfEdgeMesh &mesh, double factor, bool preserveBoundary){
    const std::vector<HEVert> &vertices=mesh.getVertices();
    std::vector<Vec3d> oldPositions;
    std::vector<Vec3d> newPositions;

    oldPositions.reserve(vertices.size());
    newPositions.reserve(vertices.size());

    for(auto v:vertices){
        Vec3d p(v.x, v.y, v.z);
        oldPositions.push_back(p);
        newPositions.push_back(p);
    }

    for(int i=0; i<vertices.size(); i++){
        if(preserveBoundary&&mesh.isBoundaryVertex(i)){
            continue;
        }

        std::vector<int> neighbors=mesh.vertexNeighbors(i);

        if(neighbors.empty()){
            continue;
        }

        Vec3d avg(0, 0, 0);
        int count=0;

        for(auto id:neighbors){
            if(id<0||id>=oldPositions.size()){
                continue;
            }

            avg+=oldPositions.at(id);
            count++;
        }

        if(count==0){
            continue;
        }

        avg/=static_cast<double>(count);
        Vec3d p=oldPositions.at(i);
        newPositions.at(i)=p+factor*(avg-p);
    }

    for(int i=0; i<newPositions.size(); i++){
        mesh.setVertexPosition(i, newPositions.at(i));
    }
}
}

void GeometryProcessing::uniformLaplacianSmooth(HalfEdgeMesh &mesh, double lambda, int iterations, bool preserveBoundary){
    if(lambda<=0.0||iterations<=0){
        return;
    }

    for(int iter=0; iter<iterations; iter++){
        laplacianStep(mesh, lambda, preserveBoundary);
    }
}

void GeometryProcessing::taubinSmooth(HalfEdgeMesh &mesh, double lambda, double mu, int iterations, bool preserveBoundary){
    if(lambda<=0.0||mu>=0.0||iterations<=0){
        return;
    }

    for(int iter=0; iter<iterations; iter++){
        laplacianStep(mesh, lambda, preserveBoundary);
        laplacianStep(mesh, mu, preserveBoundary);
    }
}