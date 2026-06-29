#include"HalfEdgeMesh.h"
#include"unordered_map"
#include<iostream>
namespace {
    struct EdgeKey {
        int from;
        int to;
        bool operator==(const EdgeKey& other) const {
            return from == other.from && to == other.to;
        }
    };
    struct EdgeKeyHash {
        std::size_t operator()(const EdgeKey& key) const {
            return std::hash<int>()(key.from) ^ (std::hash<int>()(key.to) << 1);
        }
    };
}
bool HalfEdgeMesh::buildFromMesh(const Mesh &mesh){
    vertices.clear();
    edges.clear();
    faces.clear();
    texcoords.clear();

    texcoords = mesh.texcoords;
    // for(int i=0;i<3;i++){
    //     std::cout<<texcoords[i].x()<<texcoords[i].y()<<'\n';
    // }
    std::unordered_map<EdgeKey, int, EdgeKeyHash> edgeMap;
    //建立顶点
    for(auto v:mesh.positions){
        vertices.push_back({v.x(),v.y(),v.z()});
    }
    //建立面、边对
    for(auto f:mesh.faces){

        int prev=-1;
        int start=-1;
        int prevEdgeId=-1;
        int startEdgeId=-1;
        int vt0=-1;
        for(auto objIndex:f.indices){
            if(start==-1){
                start=objIndex.v;
                vt0=objIndex.vt;
            } else {
                HEEdge edge;
                int faceId=faces.size();
                int edgeId=edges.size();
                int vertId=objIndex.v;
                
                if (vertices.at(prev).edge == -1) {
                    vertices.at(prev).edge = edgeId;
                }

                edge.face=faceId;
                edge.vert=vertId;
                edge.texcoord=objIndex.vt;
                // std::cout<<objIndex.vt<<'\n';
                
                if(prevEdgeId!=-1){
                    HEEdge& prevEdge=edges.at(prevEdgeId);
                    prevEdge.next=edgeId;
                    edge.prev=prevEdgeId;
                }

                if(edgeMap.find({vertId,prev})!=edgeMap.end()){
                    int pairEdgeId=edgeMap.at({vertId,prev});
                    HEEdge& pairEdge=edges.at(pairEdgeId);
                    edge.pair=pairEdgeId;
                    pairEdge.pair=edgeId;
                } else {
                    edgeMap.insert({{prev,vertId},edgeId});
                }

                edges.push_back(edge);
               
                if(startEdgeId==-1){
                    startEdgeId=edgeId;
                }
                
                prevEdgeId=edgeId;
            }
            prev=objIndex.v;
        }
        HEEdge edge;
        int faceId=faces.size();
        int edgeId=edges.size();
        int vertId=start;

        if (vertices.at(prev).edge == -1) {
            vertices.at(prev).edge = edgeId;
        }

        edge.face=faceId;
        edge.vert=vertId;
        edge.texcoord=vt0;
                
        if(prevEdgeId!=-1){
            HEEdge& prevEdge=edges.at(prevEdgeId);
            prevEdge.next=edgeId;
            edge.prev=prevEdgeId;
        }
        edge.next=startEdgeId;
        HEEdge &startEdge=edges.at(startEdgeId);
        startEdge.prev=edgeId;

        if(edgeMap.find({vertId,prev})!=edgeMap.end()){
            int pairEdgeId=edgeMap.at({vertId,prev});
            HEEdge& pairEdge=edges.at(pairEdgeId);
            edge.pair=pairEdgeId;
            pairEdge.pair=edgeId;
        } else {
            edgeMap.insert({{prev,vertId},edgeId});
        }

        edges.push_back(edge);

        HEFace face;
        face.edge=startEdgeId;
        faces.push_back(face);
    }

    std::cout<<"网格构建完成 "<<"顶点: "<<vertices.size()<<" 半边: "<<edges.size()<<" 面: "<<faces.size()<<'\n';
    return true;
}

bool HalfEdgeMesh::validate() const {
    return validateReport().ok;
}

ValidationReport HalfEdgeMesh::validateReport() const {
    ValidationReport report;

    report.vertexCount=vertices.size();
    report.halfedgeCount=edges.size();
    report.faceCount=faces.size();

    std::unordered_map<EdgeKey, int, EdgeKeyHash> edgeUseCount;
    std::vector<int> boundaryVertexMark(vertices.size(), 0);

    for(int i=0; i<vertices.size(); i++){
        const HEVert &v=vertices.at(i);
        if(v.edge==-1){
            report.isolatedVertex++;
        } else if(v.edge<0||v.edge>=edges.size()){
            report.invalidEdgeReference++;
        }
    }

    for(int i=0; i<edges.size(); i++){
        const HEEdge &e=edges.at(i);

        bool edgeOk=true;

        if(e.vert<0||e.vert>=vertices.size()){
            report.invalidVertexReference++;
            edgeOk=false;
        }

        if(e.face<0||e.face>=faces.size()){
            report.invalidFaceReference++;
            edgeOk=false;
        }

        if(e.next<0||e.next>=edges.size()){
            report.invalidEdgeReference++;
            report.invalidNextPrev++;
            edgeOk=false;
        }

        if(e.prev<0||e.prev>=edges.size()){
            report.invalidEdgeReference++;
            report.invalidNextPrev++;
            edgeOk=false;
        }

        if(e.next>=0&&e.next<edges.size()&&edges.at(e.next).prev!=i){
            report.invalidNextPrev++;
        }

        if(e.prev>=0&&e.prev<edges.size()&&edges.at(e.prev).next!=i){
            report.invalidNextPrev++;
        }

        if(e.pair!=-1){
            if(e.pair<0||e.pair>=edges.size()){
                report.invalidEdgeReference++;
                report.invalidPair++;
            } else if(edges.at(e.pair).pair!=i){
                report.invalidPair++;
            }
        } else {
            report.boundaryEdgeCount++;
        }

        if(edgeOk&&e.prev>=0&&e.prev<edges.size()){
            int a=edges.at(e.prev).vert;
            int b=e.vert;

            if(a>=0&&a<vertices.size()&&b>=0&&b<vertices.size()){
                if(a>b){
                    int t=a;
                    a=b;
                    b=t;
                }

                edgeUseCount[{a, b}]++;

                if(e.pair==-1){
                    boundaryVertexMark.at(a)=1;
                    boundaryVertexMark.at(b)=1;
                }
            }
        }
    }

    for(auto item:edgeUseCount){
        report.undirectedEdgeCount++;
        if(item.second>2){
            report.nonManifoldEdge++;
        }
    }

    for(auto mark:boundaryVertexMark){
        if(mark){
            report.boundaryVertexCount++;
        }
    }

    for(int i=0; i<faces.size(); i++){
        const HEFace &f=faces.at(i);

        if(f.edge<0||f.edge>=edges.size()){
            report.invalidEdgeReference++;
            continue;
        }

        int start=f.edge;
        int cur=start;
        int count=0;
        bool badLoop=false;

        while(true){
            if(cur<0||cur>=edges.size()){
                report.invalidEdgeReference++;
                badLoop=true;
                break;
            }

            const HEEdge &e=edges.at(cur);

            if(e.face!=i){
                report.invalidFaceReference++;
                badLoop=true;
            }

            count++;
            cur=e.next;

            if(cur==start){
                break;
            }

            if(count>edges.size()){
                report.invalidNextPrev++;
                badLoop=true;
                break;
            }
        }

        if(count<3||badLoop){
            report.degenerateFace++;
        }
    }

    if(report.invalidVertexReference>0||
       report.invalidEdgeReference>0||
       report.invalidFaceReference>0||
       report.invalidNextPrev>0||
       report.invalidPair>0||
       report.degenerateFace>0||
       report.nonManifoldEdge>0){
        report.ok=false;
    }

    return report;
}

void HalfEdgeMesh::printValidationReport(const ValidationReport &report) const {
    std::cout<<"拓扑报告"<<'\n';
    std::cout<<"顶点 "<<report.vertexCount<<'\n';
    std::cout<<"半边 "<<report.halfedgeCount<<'\n';
    std::cout<<"面 "<<report.faceCount<<'\n';
    std::cout<<"无向边 "<<report.undirectedEdgeCount<<'\n';
    std::cout<<"边界边 "<<report.boundaryEdgeCount<<'\n';
    std::cout<<"边界点 "<<report.boundaryVertexCount<<'\n';

    if(report.invalidVertexReference>0){
        std::cout<<"非法顶点引用 "<<report.invalidVertexReference<<'\n';
    }

    if(report.invalidEdgeReference>0){
        std::cout<<"非法边引用 "<<report.invalidEdgeReference<<'\n';
    }

    if(report.invalidFaceReference>0){
        std::cout<<"非法面引用 "<<report.invalidFaceReference<<'\n';
    }

    if(report.invalidNextPrev>0){
        std::cout<<"next/prev错误 "<<report.invalidNextPrev<<'\n';
    }

    if(report.invalidPair>0){
        std::cout<<"pair错误 "<<report.invalidPair<<'\n';
    }

    if(report.degenerateFace>0){
        std::cout<<"坍缩面 "<<report.degenerateFace<<'\n';
    }

    if(report.nonManifoldEdge>0){
        std::cout<<"非流形边 "<<report.nonManifoldEdge<<'\n';
    }

    if(report.isolatedVertex>0){
        std::cout<<"孤立点 "<<report.isolatedVertex<<'\n';
    }

    std::cout<<"结果 "<<report.ok<<'\n';
}

std::vector<int> HalfEdgeMesh::faceVertices(int faceId) const {
    const HEFace &f=faces.at(faceId);
    int edgeId=f.edge;

    std::vector<int> v;
    
    int start=edgeId;
    int cur=edgeId;

    while(true){
        const HEEdge &edge=edges.at(cur);
        v.push_back(edge.vert);
        cur=nextEdge(cur);
        if(cur==start){
            break;
        }
    }
    return v;
}

std::vector<int> HalfEdgeMesh::faceEdges(int faceId) const {
    std::vector<int> result;

    if(faceId<0||faceId>=faces.size()){
        return result;
    }

    int start=faces.at(faceId).edge;
    if(start<0||start>=edges.size()){
        return result;
    }

    int cur=start;

    while(true){
        result.push_back(cur);
        cur=edges.at(cur).next;
        if(cur==start){
            break;
        }
        if(cur<0||cur>=edges.size()||result.size()>edges.size()){
            result.clear();
            return result;
        }
    }

    return result;
}

std::vector<int> HalfEdgeMesh::faceTexcoords(int faceId) const {
    std::vector<int> edges=faceEdges(faceId);
    std::vector<int> t;

    t.reserve(edges.size());

    for(auto edge:edges){
        t.push_back(edgeTexcoord(edge));
    }

    return t;
}

int HalfEdgeMesh::edgeOrigin(int edgeId) const {
    if(edgeId<0||edgeId>=edges.size()){
        return -1;
    }
    const HEEdge &edge=edges.at(edgeId);
    //std::cout<<edge.prev<<'\n';
    if(edge.prev<0||edge.prev>=edges.size()){
        return -1;
    }
    return edges.at(edge.prev).vert;
}

int HalfEdgeMesh::edgeTarget(int edgeId) const {
    if(edgeId<0||edgeId>=edges.size()){
        return -1;
    }
    return edges.at(edgeId).vert;
}

std::vector<int> HalfEdgeMesh::vertexNeighbors(int vertId) const {
    const HEVert &vert=vertices.at(vertId);

    std::vector<int> v;

    int startEdgeId=vert.edge;
    int cur=vert.edge;
    while(true){
        const HEEdge &edge=edges.at(cur);
        
        v.push_back(edge.vert);
        
        if(edge.pair==-1){
            break;
        }

        const HEEdge &pairEdge=edges.at(edge.pair);
        cur=pairEdge.next;

        if(cur==startEdgeId){
            break;
        }
    }
    return v;
}

std::vector<int> HalfEdgeMesh::vertexEdges(int vertId) const {
    const HEVert &vert=vertices.at(vertId);

    std::vector<int> e;

    int startEdgeId=vert.edge;
    int cur=vert.edge;
    while(true){
        const HEEdge &edge=edges.at(cur);
        
        e.push_back(cur);
        
        if(edge.pair==-1){
            break;
        }

        const HEEdge &pairEdge=edges.at(edge.pair);
        cur=pairEdge.next;

        if(cur==startEdgeId){
            break;
        }
    }
    return e;
}

std::vector<int> HalfEdgeMesh::vertexFaces(int vertId) const {
    const HEVert &vert=vertices.at(vertId);

    std::vector<int> f;

    int startEdgeId=vert.edge;
    int cur=vert.edge;
    while(true){
        const HEEdge &edge=edges.at(cur);
        
        f.push_back(edge.face);
        
        if(edge.pair==-1){
            break;
        }

        const HEEdge &pairEdge=edges.at(edge.pair);
        cur=pairEdge.next;

        if(cur==startEdgeId){
            break;
        }
    }
    return f;
}

bool HalfEdgeMesh::isBoundaryEdge(int edgeId) const {
    if(edgeId<0||edgeId>=edges.size())return false;
    return edges.at(edgeId).pair==-1;
}

bool HalfEdgeMesh::isBoundaryVertex(int vertId) const {
    if(vertId<0||vertId>=vertices.size()){
        return false;
    }
    for(int i=0; i<edges.size(); i++){
        if(edges.at(i).pair!=-1){
            continue;
        }
        int origin=edgeOrigin(i);
        int target=edgeTarget(i);

        if(origin==vertId||target==vertId){
            return true;
        }
    }
    return false;
}

std::vector<std::vector<int>> HalfEdgeMesh::boundaryLoops() const {
    std::vector<std::vector<int>> loops;
    std::vector<int> used(edges.size(), 0);

    for(int i=0; i<edges.size(); i++){
        if(!isBoundaryEdge(i)||used.at(i)){
            continue;
        }

        std::vector<int> loop;
        int start=i;
        int cur=i;

        while(true){
            if(used.at(cur)){
                break;
            }

            used.at(cur)=1;
            loop.push_back(cur);
            int target=edgeTarget(cur);
            int next=-1;

            for(int j=0; j<edges.size(); j++){
                if(!isBoundaryEdge(j)||used.at(j)){
                    continue;
                }
                if(edgeOrigin(j)==target){
                    next=j;
                    break;
                }
            }
            if(next==-1){
                break;
            }
            cur=next;
            if(cur==start){
                break;
            }
        }
        loops.push_back(loop);
    }
    return loops;
}

void HalfEdgeMesh::dumpFace(int faceId) const {
    if(faceId<0||faceId>=faces.size()){
        std::cout<<"面非法 "<<faceId<<'\n';
        return;
    }

    std::cout<<"面 "<<faceId<<'\n';

    std::vector<int> e=faceEdges(faceId);
    for(auto edgeId:e){
        const HEEdge &edge=edges.at(edgeId);
        std::cout<<"边 "<<edgeId
                 <<" 起 "<<edgeOrigin(edgeId)
                 <<" 终 "<<edgeTarget(edgeId)
                 <<" next "<<edge.next
                 <<" prev "<<edge.prev
                 <<" pair "<<edge.pair
                 <<" vt "<<edge.texcoord
                 <<'\n';
    }
}

void HalfEdgeMesh::dumpEdge(int edgeId) const {
    if(edgeId<0||edgeId>=edges.size()){
        std::cout<<"边非法 "<<edgeId<<'\n';
        return;
    }

    const HEEdge &edge=edges.at(edgeId);

    std::cout<<"边 "<<edgeId<<'\n';
    std::cout<<"起点 "<<edgeOrigin(edgeId)<<'\n';
    std::cout<<"终点 "<<edgeTarget(edgeId)<<'\n';
    std::cout<<"面 "<<edge.face<<'\n';
    std::cout<<"next "<<edge.next<<'\n';
    std::cout<<"prev "<<edge.prev<<'\n';
    std::cout<<"pair "<<edge.pair<<'\n';
    std::cout<<"vt "<<edge.texcoord<<'\n';

    if(edge.next>=0&&edge.next<edges.size()){
        std::cout<<"next.prev "<<edges.at(edge.next).prev<<'\n';
    }

    if(edge.prev>=0&&edge.prev<edges.size()){
        std::cout<<"prev.next "<<edges.at(edge.prev).next<<'\n';
    }

    if(edge.pair>=0&&edge.pair<edges.size()){
        std::cout<<"pair.pair "<<edges.at(edge.pair).pair<<'\n';
    }
}

void HalfEdgeMesh::dumpVertex(int vertId) const {
    if(vertId<0||vertId>=vertices.size()){
        std::cout<<"点非法 "<<vertId<<'\n';
        return;
    }

    const HEVert &vert=vertices.at(vertId);

    std::cout<<"点 "<<vertId<<'\n';
    std::cout<<"坐标 "<<vert.x<<" "<<vert.y<<" "<<vert.z<<'\n';
    std::cout<<"代表边 "<<vert.edge<<'\n';
    std::cout<<"边界 "<<isBoundaryVertex(vertId)<<'\n';

    std::cout<<"出边";
    for(int i=0; i<edges.size(); i++){
        if(edgeOrigin(i)==vertId){
            std::cout<<" "<<i;
        }
    }
    std::cout<<'\n';

    std::cout<<"入边";
    for(int i=0; i<edges.size(); i++){
        if(edgeTarget(i)==vertId){
            std::cout<<" "<<i;
        }
    }
    std::cout<<'\n';

    std::cout<<"关联面";
    for(int i=0; i<edges.size(); i++){
        if(edgeOrigin(i)==vertId||edgeTarget(i)==vertId){
            int faceId=edges.at(i).face;
            bool exists=false;

            for(int j=0; j<i; j++){
                if((edgeOrigin(j)==vertId||edgeTarget(j)==vertId)&&edges.at(j).face==faceId){
                    exists=true;
                    break;
                }
            }

            if(!exists){
                std::cout<<" "<<faceId;
            }
        }
    }
    std::cout<<'\n';
}