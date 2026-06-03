#pragma once

#include<vector>
#include<string>
#include<Eigen/Core>

class FrameBuffer {
public:
    FrameBuffer(int width, int height);

    int getWidth() const;
    int getHeight() const;

    void clearColor(const Eigen::Vector3d& color);
    void clearDepth(double depth);
    void clear(const Eigen::Vector3d& color, double depth);
    void setPixel(int x, int y, const Eigen::Vector3d& color);
    Eigen::Vector3d getPixel(int x, int y) const;
    double getDepth(int x, int y) const;
    void setDepth(int x, int y, double depth);
    bool depthTest(int x, int y, double depth) const;
    void savePPM(const std::string &filename) const;

private:
    int width;
    int height;

    std::vector<Eigen::Vector3d> colorBuffer;
    std::vector<double> depthBuffer;

    int index(int x, int y) const;
    bool inBounds(int x, int y) const;

};