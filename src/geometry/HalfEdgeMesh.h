#pragma once
#include"Mesh.h"
#include<vector>
struct HEVert{
    double x,y,z;
    int edge=-1;
};

struct HEEdge{
    int vert=-1;
    int pair=-1;
    int face=-1;
    int next=-1;
    int prev=-1;
};

struct HEFace{
    int edge=-1;
};

class HalfEdgeMesh {
public:
    bool buildFromMesh(const Mesh &mesh);
    const std::vector<HEVert>& getVertices() const { return vertices;};
    const std::vector<HEEdge>& getEdges() const { return edges; };
    const std::vector<HEFace>& getFaces() const { return faces; };
    int nextEdge(int e) const { return edges[e].next; }
    int pairEdge(int e) const { return edges[e].pair; }
    int edgeToVertex(int e) const { return edges[e].vert; }
    int edgeFace(int e) const { return edges[e].face; }

    bool validate() const;

    std::vector<int> faceVertices(int faceId) const;
    std::vector<int> faceEdges(int faceId) const;

    int edgeOrigin(int edgeId) const;
    int edgeTarget(int edgeId) const;
    int prevEdge(int edgeId) const; 

    std::vector<int> vertexNeighbors(int vertId) const;
    std::vector<int> vertexEdges(int vertId) const;
    std::vector<int> vertexFaces(int vertId) const;
    bool isBoundaryEdge(int edgeId) const;
    bool isBoundaryVertex(int vertId) const;

private:
    std::vector<HEVert> vertices;
    std::vector<HEEdge> edges;
    std::vector<HEFace> faces;
};



