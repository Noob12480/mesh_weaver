#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

class FrameBuffer;

class Win32Window {
public:
    Win32Window(int width, int height, const wchar_t* title);
    ~Win32Window();
    bool shouldClose() const;
    void pollEvents();
    void present(const FrameBuffer& buffer);

private:
    int width;
    int height;
    bool closed = false;
    HWND hwnd = nullptr;
    HINSTANCE hInstance = nullptr;
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};