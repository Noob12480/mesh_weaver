#include"ObjIO.h"
#include"Mesh.h"
#include<fstream>
#include<iostream>
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
            while(ss>>token){
                int pos=token.find('/');
                if(pos==-1){
                    std::cerr<<"不合法的面: "<<line<<'\n';
                    return false;
                } else {
                    int v,vt;
                    v=std::stoi(token.substr(0,pos))-1;
                    vt=std::stoi(token.substr(pos+1))-1;
                    f.indices.push_back({v,vt,0});
                }
            }
            mesh.faces.push_back(f);
        } 
    }
    std::cout<<"读取完成："<<obj_name<<" 顶点："<<mesh.positions.size()<<" 面数："<<mesh.faces.size()<<'\n';
    return true;
}
bool ObjIO::save(const std::string &filename, const Mesh &mesh){
    
    return false;
}