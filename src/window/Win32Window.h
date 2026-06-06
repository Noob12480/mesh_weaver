#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include "core/MathTypes.h"

class FrameBuffer;

struct RenderOverlay {
    Vec3d cameraPos = Vec3d(0, 0, 0);
    int visibleTriangles = 0;
    int totalTriangles = 0;
};

class Win32Window {
public:
    Win32Window(int width, int height, const wchar_t* title);
    ~Win32Window();
    bool shouldClose() const;
    void pollEvents();
    void Win32Window::present(const FrameBuffer& buffer, const RenderOverlay& overlay);

private:
    int width;
    int height;
    bool closed = false;
    HWND hwnd = nullptr;
    HINSTANCE hInstance = nullptr;
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};