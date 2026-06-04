#include "window/Win32Window.h"
#include "renderer/FrameBuffer.h"
#include "core/MathTypes.h"

#include <vector>
#include <algorithm>

LRESULT CALLBACK Win32Window::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Win32Window* window = nullptr;

    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = reinterpret_cast<Win32Window*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    } else {
        window = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    switch (msg) {
    case WM_CLOSE:
        if (window) {
            window->closed = true;
        }
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        if (window) {
            window->closed = true;
        }
        return 0;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

Win32Window::Win32Window(int width, int height, const wchar_t* title)
    : width(width), height(height) {

    hInstance = GetModuleHandleW(nullptr);

    const wchar_t* className = L"MeshLabWin32Window";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = Win32Window::WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);

    RegisterClassW(&wc);

    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    hwnd = CreateWindowExW(
        0,
        className,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        hInstance,
        this
    );

    ShowWindow(hwnd, SW_SHOW);
}

Win32Window::~Win32Window() {
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

bool Win32Window::shouldClose() const {
    return closed;
}

void Win32Window::pollEvents() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void Win32Window::present(const FrameBuffer& buffer) {
    int w = buffer.getWidth();
    int h = buffer.getHeight();

    std::vector<unsigned char> pixels(w * h * 4);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            Vec3d c = buffer.getPixel(x, y);

            unsigned char r = static_cast<unsigned char>(std::clamp(c.x(), 0.0, 1.0) * 255.0);
            unsigned char g = static_cast<unsigned char>(std::clamp(c.y(), 0.0, 1.0) * 255.0);
            unsigned char b = static_cast<unsigned char>(std::clamp(c.z(), 0.0, 1.0) * 255.0);

            int idx = (y * w + x) * 4;

            pixels[idx + 0] = b;
            pixels[idx + 1] = g;
            pixels[idx + 2] = r;
            pixels[idx + 3] = 255;
        }
    }

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    HDC dc = GetDC(hwnd);

    StretchDIBits(
        dc,
        0, 0, width, height,
        0, 0, w, h,
        pixels.data(),
        &bmi,
        DIB_RGB_COLORS,
        SRCCOPY
    );

    ReleaseDC(hwnd, dc);
}