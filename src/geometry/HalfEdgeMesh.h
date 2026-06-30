#pragma once
#include"Mesh.h"
#include<vector>
#include"core/MathTypes.h"
#include<unordered_map>
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
    int texcoord=-1;
};

struct HEFace{
    int edge=-1;
};

struct ValidationReport{
    int vertexCount=0;
    int halfedgeCount=0;
    int faceCount=0;
    int undirectedEdgeCount=0;
    int boundaryEdgeCount=0;
    int boundaryVertexCount=0;

    int invalidVertexReference=0;
    int invalidEdgeReference=0;
    int invalidFaceReference=0;
    int invalidNextPrev=0;
    int invalidPair=0;
    int degenerateFace=0;
    int nonManifoldEdge=0;
    int isolatedVertex=0;

    bool ok=true;
};

class HalfEdgeMesh {
public:
    bool buildFromMesh(const Mesh &mesh);
    const std::vector<HEVert>& getVertices() const { return vertices;};
    void setVertexPosition(int vertId, const Vec3d &pos);
    const std::vector<HEEdge>& getEdges() const { return edges; };
    const std::vector<HEFace>& getFaces() const { return faces; };
    const std::vector<Vec2d>& getTexcoords() const { return texcoords; }
    int nextEdge(int e) const { return edges[e].next; }
    int pairEdge(int e) const { return edges[e].pair; }
    int edgeToVertex(int e) const { return edges[e].vert; }
    int edgeFace(int e) const { return edges[e].face; }
    int edgeTexcoord(int e) const { return edges[e].texcoord; }

    bool validate() const;
    ValidationReport validateReport() const;
    void printValidationReport(const ValidationReport &report) const;

    std::vector<int> faceVertices(int faceId) const;
    std::vector<int> faceEdges(int faceId) const;
    std::vector<int> faceTexcoords(int faceId) const;

    int edgeOrigin(int edgeId) const;
    int edgeTarget(int edgeId) const;

    std::vector<int> vertexNeighbors(int vertId) const;
    std::vector<int> vertexEdges(int vertId) const;
    std::vector<int> vertexFaces(int vertId) const;
    bool isBoundaryEdge(int edgeId) const;
    bool isBoundaryVertex(int vertId) const;
    std::vector<std::vector<int>> boundaryLoops() const;

    void dumpFace(int faceId) const;
    void dumpEdge(int edgeId) const;
    void dumpVertex(int vertId) const;

private:
    std::vector<HEVert> vertices;
    std::vector<HEEdge> edges;
    std::vector<HEFace> faces;
    std::vector<Vec2d> texcoords;
};



