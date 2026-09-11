#include <ngin/core/DXDebugLayer.h>
#include <ngin/core/base.h>
#include <ngin/pch.h>
#include <ngin/platform/window.h>

#include <format>
#include <unordered_map>

std::unordered_map<HWND, Ngin::Window::Window*> windows;
std::vector<HWND> windowsToClose;

Ngin::Window::Window* GetWindow(HWND hwnd) {
  if (windows.contains(hwnd)) {
    return windows.at(hwnd);
  }
  return nullptr;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  Ngin::Window::Window* window = GetWindow(hwnd);
  switch (message) {
    case WM_KEYDOWN:
      if (wParam == VK_F11) {
        Ngin::logDebug(std::format("Setting window fullscreen: {}", !window->fullscreen));
        Ngin::Window::SetFullscreen(*window, !window->fullscreen);
      }
      return 0;
    case WM_CREATE:
      Ngin::logTrace("Window created :)");
      return 0;
    case WM_SIZE:
      if (lParam && (HIWORD(lParam) != window->height || LOWORD(lParam) != window->width) &&
          window != nullptr) {
        Ngin::logTrace("Resizing window");
        Ngin::Error err = Resize(*window);
        if (err.code != Ngin::ErrorCode::None)
          Ngin::logError(err.message);
      }
      return 0;
    case WM_CLOSE:
      windowsToClose.push_back(hwnd);
      return 0;

    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;

    default:
      return DefWindowProc(hwnd, message, wParam, lParam);
  }
}

namespace Ngin {
namespace Window {
Error Create(Window& window) {
  WNDCLASSEXW wc;
  HINSTANCE instance = GetModuleHandle(nullptr);
  std::wstring className(window.className.begin(), window.className.end());
  std::wstring windowName(window.name.begin(), window.name.end());

  logTrace("Setting wndClass desc");
  wc.cbSize = sizeof(wc);
  wc.style = CS_OWNDC;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = instance;
  wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  wc.hCursor = LoadCursor(0, IDC_ARROW);
  wc.hbrBackground = nullptr;
  wc.lpszMenuName = nullptr;
  wc.lpszClassName = className.c_str();
  wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

  logTrace("Registering class");
  window.w_class = RegisterClassExW(&wc);
  if (!window.w_class) {
    Ngin::logError(std::format("Creating window failed. Couldnt RegisterClass Error code {}",
        GetLastError()));
    return Error{PlatformError,
        std::format("Creating window failed. Couldnt RegisterClass Error code {}",
            GetLastError())};
  }

  logTrace("Creating windowex");
  window.hwnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW, className.c_str(),
      windowName.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, window.width,
      window.height, nullptr, nullptr, instance, nullptr);

  if (window.hwnd == nullptr) {
    Ngin::logError(std::format("Creating Window failed: Couldnt CreateWindowEx Error code {}",
        GetLastError()));
    return Error{PlatformError,
        std::format("Creating Window failed: Couldnt CreateWindowEx Error code {}",
            GetLastError())};
  }

  logTrace("Creating RHI");
  HRESULT hr = RHI::Create(window.hwnd, window.width, window.height, window.rhi);

  if (FAILED(hr)) {
    if (hr == -2147024894) {
      Ngin::logError("Shader file could not be found");
      return Error{FileNotFound, "Shader file could not be found"};
    }
    Ngin::logError(std::format("RHI creation failed: {}", hr));
    return Error{GraphicsError, std::format("RHI creation failed: {}", hr)};
  }

  windows.insert_or_assign(window.hwnd, &window);

  return Error{};
}

Error SetShow(Window& props, CmdShow showType) {
  if (props.hwnd == 0) {
    return Error{PlatformError, "Window Handle is NULL"};
  }
  ShowWindow(props.hwnd, showType);
  return Error{};
}

void CheckForClosableWindows() {
  if (windowsToClose.size() <= 0) {
    return;
  }
  for (const auto windowToClose : windowsToClose) {
    if (windows.contains(windowToClose)) {
      Close(*windows.at(windowToClose));
    }
  }
}

Error Update(Window& window) {
  if (window.hwnd == 0) {
    return Error{PlatformError, "Window handle is NULL"};
  }

  CheckForClosableWindows();

  MSG msg;

  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) {
      Close(window);
      window.hwnd = 0;
      return Error{Exit, "Window was closed"};
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  Error err = window.rhi->BeginFrame();
  if (err)
    return err;
  // window.rhi->DoDrawingAndShit
  err = window.rhi->EndFrame();
  if (err)
    return err;
  err = window.rhi->Update();
  if (err)
    return err;
  return Error{};
}

Error Resize(Window& window) {
  RECT cr;
  if (GetClientRect(window.hwnd, &cr)) {
    window.rhi->ReleaseBuffers();
    window.width = cr.right - cr.left;
    window.height = cr.bottom - cr.top;

    Error err = window.rhi->Resize(window.width, window.height);
    if (err.code)
      return err;
    return window.rhi->GetBuffers();
  }
  return Error{Unknown, "Couldnt get Client Rect for Resizing"};
}

Error SetFullscreen(Window& window, bool enable) {
  window.fullscreen = enable;
  DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  DWORD exStyle = WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW;

  if (window.fullscreen) {
    style = WS_POPUP | WS_VISIBLE;
    exStyle = WS_EX_APPWINDOW;
  }
  SetWindowLongW(window.hwnd, GWL_STYLE, style);
  SetWindowLongW(window.hwnd, GWL_EXSTYLE, exStyle);
  if (window.fullscreen) {
    HMONITOR monitor = MonitorFromWindow(window.hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (GetMonitorInfo(monitor, &monitorInfo)) {
      SetWindowPos(window.hwnd, nullptr, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
          monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
          monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top, SWP_NOZORDER);
      Resize(window);
    }
  } else {
    SetShow(window, ShowMaximized);
  }
  return Error{};
}

Error Close(Window& window) {
  if (window.hwnd) {
    DestroyWindow(window.hwnd);
    windows.erase(window.hwnd);
    window.hwnd = nullptr;
  }
  if (window.w_class) {
    std::wstring className(window.className.begin(), window.className.end());
    UnregisterClassW(className.c_str(), GetModuleHandle(nullptr));
  }
  return Error{};
}

}  // namespace Window
}  // namespace Ngin
