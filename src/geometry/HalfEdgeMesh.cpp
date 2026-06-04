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
        for(auto objIndex:f.indices){
            if(start==-1){
                start=objIndex.v;
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
    for(int i=0;i<vertices.size();i++){
        auto &v=vertices.at(i);
        if(v.edge<0||v.edge>=edges.size()){
            std::cerr<<"非法的边索引 "<<"顶点："<<i<<'\n';
            return false;
        }
    }
    //检查边的索引
    for(auto e:edges){
        if(e.vert<0||e.vert>=vertices.size()){
            std::cerr<<"非法的顶点索引"<<'\n';
            return false;
        } else if(e.face<0||e.face>=faces.size()){
            return false;
            std::cerr<<"非法的面索引"<<'\n';
        } else if(e.next<0||e.next>=edges.size()||e.prev<0||e.prev>=edges.size()){
            return false;
            std::cerr<<"非法的半边索引"<<'\n';
        }
    }
    //检查面的边索引及其闭合性
    for(auto f:faces){

        if(f.edge< 0||f.edge>=edges.size()) {
            std::cerr<<"非法的边索引"<<'\n';
            return false;
        }

        int start=f.edge;
        int cur=f.edge;
        int count=0;
        while(true){
            //形成回路
            cur=nextEdge(cur);
            if(cur==start){
                break;
            }
            count++;
            if(count>edges.size()){
                std::cerr<<"存在非法回路"<<'\n';
                return false;
            }
        }
        if(count<3){
            std::cerr<<"坍缩的面"<<'\n';
            return false;
        }
    }
    return true;
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
    std::vector<int> v=faceVertices(faceId);
    
    std::vector<int> e;

    for(auto vi:v){
        const HEVert &vert=vertices.at(vi);
        e.push_back(vert.edge);
    }
    return e;
}

int HalfEdgeMesh::edgeOrigin(int edgeId) const {
    if(edgeId<0||edgeId>edges.size()){
        return -1;
    }
    const HEEdge &edge=edges.at(edgeId);
    //std::cout<<edge.prev<<'\n';
    const HEEdge prevEdge=edges.at(edge.prev);
    return prevEdge.vert;
}

int HalfEdgeMesh::edgeTarget(int edgeId) const {
    if(edgeId<0||edgeId>edges.size()){
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
    if(vertId<0||vertId>=vertices.size())return false;
    std::vector<int> e=vertexEdges(vertId);
    for(auto edge:e){
        if(isBoundaryEdge(edge)){
            return true;
        }
    }
    return false;
}