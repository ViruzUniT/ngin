#include <ngin/core/base.h>
#include <ngin/pch.h>
#include <ngin/platform/window.h>

#include <format>
#include <unordered_map>

std::unordered_map<HWND, Ngin::Window::Window*> windows;
std::vector<HWND> windowsToClose;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_CREATE:
      Ngin::logTrace("Window created :)");
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
  window.handle = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW, className.c_str(),
      windowName.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, window.width,
      window.height, nullptr, nullptr, instance, nullptr);

  if (window.handle == nullptr) {
    Ngin::logError(std::format("Creating Window failed: Couldnt CreateWindowEx Error code {}",
        GetLastError()));
    return Error{PlatformError,
        std::format("Creating Window failed: Couldnt CreateWindowEx Error code {}",
            GetLastError())};
  }

  logTrace("Creating RHI");
  HRESULT hr = RHI::Create(window.handle, window.width, window.height, window.rhi);

  if (FAILED(hr)) {
    if (hr == -2147024894) {
      Ngin::logError("Shader file could not be found");
      return Error{FileNotFound, "Shader file could not be found"};
    }
    Ngin::logError(std::format("RHI creation failed: {}", hr));
    return Error{GraphicsError, std::format("RHI creation failed: {}", hr)};
  }

  windows.insert_or_assign(window.handle, &window);

  return Error{};
}

Error SetShow(Window& props, CmdShow shouldShow) {
  if (props.handle == 0) {
    return Error{PlatformError, "Window Handle is NULL"};
  }
  ShowWindow(props.handle, shouldShow);
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
  if (window.handle == 0) {
    return Error{PlatformError, "Window handle is NULL"};
  }

  CheckForClosableWindows();

  MSG msg;

  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) {
      Close(window);
      window.handle = 0;
      return Error{Exit, "Window was closed"};
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  window.rhi->Update();
  return Error{};
}

Error Close(Window& window) {
  if (window.handle) {
    DestroyWindow(window.handle);
    windows.erase(window.handle);
    window.handle = nullptr;
  }
  if (window.w_class) {
    std::wstring className(window.className.begin(), window.className.end());
    UnregisterClassW(className.c_str(), GetModuleHandle(nullptr));
  }
  return Error{};
}
}  // namespace Window
}  // namespace Ngin
