#include <ngin/core/base.h>
#include <ngin/pch.h>
#include <ngin/platform/window.h>

#include <format>

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_CREATE:
      Ngin::logInfo("Window created :)");
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
ErrorCode Create(Window& windowProps) {
  WNDCLASS wc;
  HINSTANCE instance = GetModuleHandleA(nullptr);

  wc.style = CS_VREDRAW | CS_HREDRAW;
  wc.lpszClassName = windowProps.className.c_str();
  wc.lpfnWndProc = WndProc;
  wc.hInstance = instance;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hCursor = LoadCursorA(0, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wc.lpszMenuName = 0;
  wc.hIcon = 0;

  if (!RegisterClassA(&wc)) {
    Ngin::logError(std::format("Creating window failed. Couldnt RegisterClass Error code {}",
        GetLastError()));
    return ErrorCode::PlatformError;
  }

  windowProps.windowHandle = CreateWindowExA(0, windowProps.className.c_str(),
      windowProps.name.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
      windowProps.width, windowProps.height, NULL, NULL, instance, NULL);

  if (windowProps.windowHandle == nullptr) {
    Ngin::logError(std::format("Creating Window failed: Couldnt CreateWindowEx Error code {}",
        GetLastError()));
    return ErrorCode::PlatformError;
  }

  HRESULT hr = RHI::Create(windowProps.windowHandle, windowProps.width, windowProps.height,
      windowProps.rhi);

  if (FAILED(hr)) {
    if (hr == -2147024894) {
      Ngin::logError("Shader file could not be found");
      return ErrorCode::FileNotFound;
    }
    Ngin::logError(std::format("RHI creation failed: {}", hr));
    return ErrorCode::GraphicsError;
  }

  return ErrorCode::None;
}

ErrorCode SetShow(Window& props, CmdShow shouldShow) {
  if (props.windowHandle == 0) {
    return ErrorCode::PlatformError;
  }
  ShowWindow(props.windowHandle, shouldShow);
  return ErrorCode::None;
}

ErrorCode Update(Window& props) {
  if (props.windowHandle == 0) {
    return ErrorCode::PlatformError;
  }

  MSG msg;

  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) {
      props.windowHandle = 0;
      return ErrorCode::PlatformError;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return ErrorCode::None;
}
}  // namespace Window
}  // namespace Ngin
