#include <ngin/core/base.h>
#include <ngin/pch.h>
#include <ngin/platform/window.h>

#include <format>

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_CREATE:
      Ngin::logTrace("Window created :)");
      return 0;
    case WM_CLOSE:
      DestroyWindow(hwnd);
      // props.windowHandle = 0;
      return 0;

    case WM_DESTROY:
      PostQuitMessage(0);
      // props.windowHandle = 0;
      return 0;

    default:
      return DefWindowProc(hwnd, message, wParam, lParam);
  }
}

namespace Ngin {
namespace Window {
Error Create(Window& windowProps) {
  WNDCLASS wc;
  HINSTANCE instance = GetModuleHandle(nullptr);

  // wc.style = CS_VREDRAW | CS_HREDRAW;
  wc.style = CS_OWNDC;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = instance;
  wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  wc.hCursor = LoadCursor(0, IDC_ARROW);
  wc.hbrBackground = nullptr;
  wc.lpszMenuName = nullptr;
  wc.lpszClassName = windowProps.className.c_str();
  wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

  if (!RegisterClassA(&wc)) {
    Ngin::logError(std::format("Creating window failed. Couldnt RegisterClass Error code {}",
        GetLastError()));
    return Error{PlatformError,
        std::format("Creating window failed. Couldnt RegisterClass Error code {}",
            GetLastError())};
  }

  windowProps.handle = CreateWindowExA(0, windowProps.className.c_str(), windowProps.name.c_str(),
      WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, windowProps.width, windowProps.height,
      NULL, NULL, instance, NULL);

  if (windowProps.handle == nullptr) {
    Ngin::logError(std::format("Creating Window failed: Couldnt CreateWindowEx Error code {}",
        GetLastError()));
    return Error{PlatformError,
        std::format("Creating Window failed: Couldnt CreateWindowEx Error code {}",
            GetLastError())};
  }

  HRESULT hr =
      RHI::Create(windowProps.handle, windowProps.width, windowProps.height, windowProps.rhi);

  if (FAILED(hr)) {
    if (hr == -2147024894) {
      Ngin::logError("Shader file could not be found");
      return Error{FileNotFound, "Shader file could not be found"};
    }
    Ngin::logError(std::format("RHI creation failed: {}", hr));
    return Error{GraphicsError, std::format("RHI creation failed: {}", hr)};
  }

  return Error{};
}

Error SetShow(Window& props, CmdShow shouldShow) {
  if (props.handle == 0) {
    return Error{PlatformError, "Window Handle is NULL"};
  }
  ShowWindow(props.handle, shouldShow);
  return Error{};
}

Error Update(Window& window) {
  if (window.handle == 0) {
    return Error{PlatformError, "Window handle is NULL"};
  }

  MSG msg;

  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) {
      window.handle = 0;
      return Error{Exit, "Window was closed"};
    }
    window.rhi->Update();
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return Error{};
}
}  // namespace Window
}  // namespace Ngin
