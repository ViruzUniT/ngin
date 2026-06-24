#include <ngin/core/base.h>
#include <ngin/pch.h>
#include <ngin/platform/window.h>

#include <format>

Ngin::Window::Properties props;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_CREATE:
      Ngin::logInfo("Window created :)");
      return 0;
    case WM_CLOSE:
      DestroyWindow(hwnd);
      props.windowHandle = 0;
      return 0;

    case WM_DESTROY:
      PostQuitMessage(0);
      props.windowHandle = 0;
      return 0;

    default:
      return DefWindowProc(hwnd, message, wParam, lParam);
  }
}

namespace Ngin {
namespace Window {
ErrorCode Create(Properties const& windowProps) {
  props = windowProps;

  WNDCLASS wc;
  HINSTANCE instance = GetModuleHandleA(nullptr);

  wc.style = CS_VREDRAW | CS_HREDRAW;
  wc.lpszClassName = props.className.c_str();
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

  props.windowHandle =
      CreateWindowExA(0, props.className.c_str(), props.name.c_str(), WS_OVERLAPPEDWINDOW,
          CW_USEDEFAULT, CW_USEDEFAULT, props.width, props.height, NULL, NULL, instance, NULL);

  if (props.windowHandle == nullptr) {
    Ngin::logError(std::format("Creating Window failed: Couldnt CreateWindowEx Error code {}",
        GetLastError()));
    return ErrorCode::PlatformError;
  }

  return ErrorCode::None;
}

ErrorCode SetShow(CmdShow shouldShow) {
  if (props.windowHandle == 0) {
    return ErrorCode::PlatformError;
  }
  ShowWindow(props.windowHandle, shouldShow);
  return ErrorCode::None;
}

ErrorCode Update() {
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
