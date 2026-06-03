#include"FrameBuffer.h"
#include <fstream>
#include <iostream>
#include <algorithm>
FrameBuffer::FrameBuffer(int width, int height) : width(width), height(height){
    colorBuffer.resize(width*height);
    depthBuffer.resize(width*height);
}

int FrameBuffer::getWidth() const{
    return width;
}
int FrameBuffer::getHeight() const{
    return height;
}

void FrameBuffer::clearColor(const Eigen::Vector3d& color){
    std::fill(colorBuffer.begin(),colorBuffer.end(),color);
}
void FrameBuffer::clearDepth(double depth){
    std::fill(depthBuffer.begin(),depthBuffer.end(),depth);
}
void FrameBuffer::clear(const Eigen::Vector3d& color, double depth){
    clearColor(color);
    clearDepth(depth);
}
void FrameBuffer::setPixel(int x, int y, const Eigen::Vector3d& color){
    colorBuffer[index(x,y)]=color;
}
Eigen::Vector3d FrameBuffer::getPixel(int x, int y) const{
    return colorBuffer[index(x,y)];
}
void FrameBuffer::setDepth(int x, int y, double depth){
    depthBuffer[index(x,y)]=depth;
}
double FrameBuffer::getDepth(int x, int y) const{
    return depthBuffer[index(x,y)];
}
bool FrameBuffer::depthTest(int x, int y, double depth) const{
    if(!inBounds(x,y))return false;
    if(depthBuffer[index(x,y)]>depth){
        return true;
    }
    return false;
}
void FrameBuffer::savePPM(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "无法保存图片: " << filename << '\n';
        return;
    }
    out << "P3\n";
    out << width << " " << height << "\n";
    out << "255\n";
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const auto& c = colorBuffer[y * width + x];
            int r = static_cast<int>(std::clamp(c.x(), 0.0, 1.0) * 255.0);
            int g = static_cast<int>(std::clamp(c.y(), 0.0, 1.0) * 255.0);
            int b = static_cast<int>(std::clamp(c.z(), 0.0, 1.0) * 255.0);
            out << r << ' ' << g << ' ' << b << ' ';
        }
        out << '\n';
    }
}
int FrameBuffer::index(int x, int y) const{
    return y*width+x;
}
bool FrameBuffer::inBounds(int x, int y) const{
    return x>=0&&x<=width&&y>=0&&y<=height;
}