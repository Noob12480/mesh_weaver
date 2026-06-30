#include"ObjIO.h"
#include<fstream>
#include<iostream>
ObjIndex parseObjIndex(const std::string& token) {
    ObjIndex idx;
    idx.v = -1;
    idx.vt = -1;
    idx.vn = -1;
    std::stringstream ss(token);
    std::string part;
    //v
    if(std::getline(ss, part, '/')){
        if (!part.empty()) idx.v = std::stoi(part) - 1;
    }
    //vt
    if(std::getline(ss, part, '/')){
        if (!part.empty()) idx.vt = std::stoi(part) - 1;
    }
    //vn
    if(std::getline(ss, part, '/')){
        if (!part.empty()) idx.vn = std::stoi(part) - 1;
    }
    return idx;
}
bool ObjIO::load(const std::string &filename, Mesh &mesh){
    
    std::ifstream in(filename);
    if(!in.is_open()){
        std::cerr<<"文件打开失败"<<'\n';
        return false;
    }

    std::string line;
    std::string obj_name;
    while(getline(in, line)){
        std::stringstream ss(line);

        std::string symble;
        ss>>symble;
        if(symble=="o"){
            ss>>obj_name;
        } else if(symble=="v"){
            double x,y,z;
            ss>>x>>y>>z;
            Eigen::Vector3d pos(x,y,z);
            mesh.positions.push_back(pos);
        } else if(symble=="vt"){
            double u,v;
            ss>>u>>v;
            Eigen::Vector2d coord(u,v);
            mesh.texcoords.push_back(coord);
        } else if(symble=="vn"){
            double x,y,z;
            ss>>x>>y>>z;
            Eigen::Vector3d pos(x,y,z);
            mesh.normals.push_back(pos);
        } else if(symble=="f"){
            std::string token;
            Face f;
            while (ss >> token) {
                ObjIndex idx = parseObjIndex(token);
                if (idx.v < 0) {
                    std::cerr << "不合法的面索引: " << line << '\n';
                    return false;
                }
                //std::cout<<idx.vt<<'\n';
                f.indices.push_back(idx);
            }
            mesh.faces.push_back(f);
        } 
    }
    std::cout<<"读取完成："<<obj_name<<" 顶点："<<mesh.positions.size()<<" 面数："<<mesh.faces.size()<<'\n';
    return true;
}
bool ObjIO::save(const std::string &filename, const Mesh &mesh){
    std::ofstream out(filename);
    if(!out.is_open()){
        std::cerr<<"文件保存失败"<<'\n';
        return false;
    }
    for(auto p:mesh.positions){
        out<<"v "<<p.x()<<" "<<p.y()<<" "<<p.z()<<'\n';
    }
    for(auto uv:mesh.texcoords){
        out<<"vt "<<uv.x()<<" "<<uv.y()<<'\n';
    }
    for(auto n:mesh.normals){
        out<<"vn "<<n.x()<<" "<<n.y()<<" "<<n.z()<<'\n';
    }
    for(auto face:mesh.faces){
        out<<"f";

        for(auto idx:face.indices){
            out<<" "<<idx.v+1;

            if(idx.vt>=0||idx.vn>=0){
                out<<"/";

                if(idx.vt>=0){
                    out<<idx.vt+1;
                }

                if(idx.vn>=0){
                    out<<"/"<<idx.vn+1;
                }
            }
        }
        out<<'\n';
    }
    std::cout<<"保存完成："<<filename<<'\n';
    return true;
}
bool ObjIO::save(const std::string &filename, const HalfEdgeMesh &mesh){
    std::ofstream out(filename);
    if(!out.is_open()){
        std::cerr<<"文件保存失败"<<'\n';
        return false;
    }

    const std::vector<HEVert> &vertices=mesh.getVertices();
    const std::vector<Vec2d> &texcoords=mesh.getTexcoords();

    for(auto v:vertices){
        out<<"v "<<v.x<<" "<<v.y<<" "<<v.z<<'\n';
    }

    for(auto uv:texcoords){
        out<<"vt "<<uv.x()<<" "<<uv.y()<<'\n';
    }

    for(int i=0; i<mesh.getFaces().size(); i++){
        std::vector<int> verts=mesh.faceVertices(i);
        std::vector<int> uvs=mesh.faceTexcoords(i);
        if(verts.size()<3){
            continue;
        }

        out<<"f";
        for(int j=0; j<verts.size(); j++){
            int v=verts.at(j);
            int vt=-1;
            if(j<uvs.size()){
                vt=uvs.at(j);
            }
            out<<" "<<v+1;

            if(vt>=0){
                out<<"/"<<vt+1;
            }
        }
        out<<'\n';
    }
    std::cout<<"保存完成："<<filename<<'\n';
    return true;
}