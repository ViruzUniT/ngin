#include <ngin/core/base.h>
#include <ngin/platform/window.h>
#include <windows.h>

#include <format>

HWND hwnd = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_CREATE:
      Ngin::logInfo("Window created :)");
      return 0;
    case WM_CLOSE:
      DestroyWindow(hwnd);
      hwnd = 0;
      return 0;

    case WM_DESTROY:
      PostQuitMessage(0);
      hwnd = 0;
      return 0;

    default:
      return DefWindowProc(hwnd, message, wParam, lParam);
  }
}

namespace Ngin {
namespace Window {
ErrorCode Create(Properties const& props) {
  WNDCLASS wc;
  wc.style = CS_VREDRAW | CS_HREDRAW;
  wc.lpszClassName = props.name.data();
  wc.lpfnWndProc = WndProc;
  wc.hInstance = nullptr;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hCursor = LoadCursorA(0, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wc.lpszMenuName = 0;
  wc.hIcon = 0;

  if (!RegisterClass(&wc)) {
    Ngin::logError(std::format("Creating window failed. Couldnt RegisterClass Error code {}",
        GetLastError()));
    return ErrorCode::PlatformError;
  }

  hwnd = CreateWindowEx(0, props.name.data(), props.name.data(), WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, props.width, props.height, NULL, NULL, nullptr, NULL);

  if (hwnd == 0) {
    Ngin::logError(std::format("Creating Window failed: Couldnt CreateWindowEx Error code {}",
        GetLastError()));
    return ErrorCode::PlatformError;
  }

  return ErrorCode::None;
}

ErrorCode SetShow(CmdShow shouldShow) {
  if (hwnd == 0) {
    return ErrorCode::PlatformError;
  }
  ShowWindow(hwnd, SW_SHOW);
  return ErrorCode::None;
}

ErrorCode Update() {
  if (hwnd == 0) {
    return ErrorCode::PlatformError;
  }

  MSG msg;

  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) {
      hwnd = 0;
      return ErrorCode::PlatformError;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return ErrorCode::None;
}
}  // namespace Window
}  // namespace Ngin
