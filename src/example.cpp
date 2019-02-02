#include <cassert>
#include <iostream>
#include <thread>
#include <random>
#include <algorithm>
#include <string>
#include <memory>
#include <atomic>
#include <cstdint>
#include <iomanip>
#include <array>
#include <numeric>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// The Windowsx.h header these are supposed to be in doesn't seem to work
#define GET_X_LPARM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARM(lp) ((int)(short)HIWORD(lp))

#include <gl/gl.h>
#include <glm/glm.hpp>

#include "timer.hpp"
#include "queue.hpp"

enum class EventType
{
    Resize,
    MouseMove
};

struct ResizeData
{
    int width;
    int height;
};

struct MouseData
{
    int x;
    int y;
};

struct EventData
{
    union
    {
        ResizeData win;
        MouseData mouse;
    };
    EventType type;
    double timestamp;
};

double fpsTime = 0.0;
std::atomic<bool> running = false;
long mytime = 0;
int count = 0;
long lastMsgTime = 0;
FpsTimer eTimer;
constexpr int queueSize = 32;

QueueSPSC<EventData, queueSize> inputQueue;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    long curTime = GetMessageTime();
    mytime += (curTime - lastMsgTime);
    ++count;
    lastMsgTime = curTime;
    switch (message)
    {
        case WM_MOUSEMOVE: {
            MouseData eventPayload {GET_X_LPARM(lParam), GET_Y_LPARM(lParam)};
            EventData event;
            event.mouse = eventPayload;
            event.type = EventType::MouseMove;
            event.timestamp = eTimer.get_time();

            while (inputQueue.push(event));

            break;
        }
        case WM_SIZING:
        {
            LPRECT lpRect = (LPRECT)lParam;

            ResizeData eventPayload {lpRect->bottom - lpRect->top, lpRect->right - lpRect->left};
            
            EventData event;
            event.win = eventPayload;
            event.type = EventType::Resize;
            event.timestamp = eTimer.get_time();

            while (inputQueue.push(event));

            break;
        }
        case WM_DESTROY:
            running = false;
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

static HWND hwnd; // hwnd should never be written to after assignment

//PointEmitter emitter;
void render_thread()
{
    std::cout.setf(std::ios::fixed);
    std::cout << std::setprecision(0);

    assert(hwnd != nullptr);

    // setup render thread
    
    HDC deviceContext = GetDC(hwnd);

    assert(deviceContext != nullptr);

    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,             //The kind of framebuffer. RGBA or palette.
        32,                        //Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                        //Number of bits for the depthbuffer
        8,                         //Number of bits for the stencilbuffer
        0,                         //Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    int pixelFormat = ChoosePixelFormat(deviceContext, &pfd);
    SetPixelFormat(deviceContext, pixelFormat, &pfd);

    HGLRC context = wglCreateContext(deviceContext);

    assert(context != nullptr);
    std::cout << "context created" << std::endl;
    wglMakeCurrent(deviceContext, context);

    std::cout.precision(5);

    glViewport(0, 0, 1920, 1080);    
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glOrtho(0, 1920, 1080, 0, -1.0, 1.0);

    FpsTimer timer;
    double tick;
    glm::vec2 mousePosCurr = {0, 0};
    glm::vec2 mousePosLast = {0,0};

    std::vector<EventData> events;

    while (running) {

        // Update
        tick = timer.tick();
        fpsTime += tick;

        // No data processing should happen within the queue loop
        // Because we will fall behind and effectively block indefinitely.
        EventData data;
        while (inputQueue.pop(data))
        {
            events.push_back(data);
        }

        std::cout << events.size() << " recieved events" << std::endl;
        // Actually process events
        for (auto &event : events)
        {
            if (event.type == EventType::MouseMove)
            {
                mousePosLast = mousePosCurr;
                mousePosCurr = glm::vec2{event.mouse.x, event.mouse.y};
            }
            else if (event.type == EventType::Resize)
            {
                std::cout << "Resize" << event.win.width << " " << event.win.height << std::endl;
                glViewport(0, 0, event.win.width, event.win.height);    
                glClearColor(0.5, 0.5, 0.5, 1.0);
                glOrtho(0, event.win.width, event.win.height, 0, -1.0, 1.0);
            }
        }
        events.clear();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        SwapBuffers(deviceContext);

        if (fpsTime >= 0.5) {
            std::cout << "FPS: " << timer.get_fps() << std::endl;
            fpsTime = 0;
        }
    }
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(context);
    ReleaseDC(hwnd, deviceContext);
}

void create_window()
{
    // setup Win32 Window
    WNDCLASS wndClass;
    wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = GetModuleHandle(nullptr);
    wndClass.hIcon = LoadIcon(nullptr, IDI_INFORMATION);
    wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndClass.hbrBackground = nullptr;
    wndClass.lpszMenuName = nullptr;
    wndClass.lpszClassName = "speedywindow9000";

    ATOM clsReg = RegisterClass(&wndClass);

    hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
        wndClass.lpszClassName,
        "Speed Test",
        WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1920, 1080,
        nullptr, nullptr,
        wndClass.hInstance,
        nullptr);

    assert(hwnd != nullptr);

    running = true;
    
    std::thread renderThread(render_thread);

    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    MSG msg;
    double fpsTime = 0.0;
    std::cout.precision(5);
    while (GetMessage(&msg, nullptr, 0, 0) != 0)
    {
        fpsTime += eTimer.tick();
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (fpsTime >= 2.0) {
            std::cout << "event LPS: " << eTimer.get_fps() << std::endl;
            std::cout << "avg event time: " << (mytime/(count*1.0)) << std::endl;
            fpsTime = 0;
            mytime = 0;
            count = 0;
        }
    }
    renderThread.join();
}

int main()
{
    create_window();
    return 0;
}