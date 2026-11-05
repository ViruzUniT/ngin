#include <ngin/core/DXDebugLayer.h>
#include <ngin/core/base.h>
#include <ngin/pch.h>
#include <ngin/platform/window.h>

#include <format>

std::unordered_map<HWND, Ngin::Window*> Ngin::Window::windows;
std::vector<HWND> Ngin::Window::windowsToClose;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  Ngin::Window* window = Ngin::Window::GetWindow(hwnd);
  switch (message) {
    case WM_KEYDOWN:
      if (wParam == VK_F11) {
        Ngin::logDebug(std::format("Setting window fullscreen: {}", !window->fullscreen));
        window->SetFullscreen(!window->fullscreen);
      }
      return 0;
    case WM_CREATE:
      Ngin::logTrace("Window created :)");
      return 0;
    case WM_SIZE:
      if (lParam &&
          (HIWORD(lParam) != window->dimensions.y || LOWORD(lParam) != window->dimensions.x) &&
          window != nullptr) {
        Ngin::logTrace("Resizing window");
        Ngin::Error err = window->Resize();
        if (err.code != Ngin::ErrorCode::None)
          Ngin::logError(err.message);
      }
      return 0;
    case WM_CLOSE:
      Ngin::Window::windowsToClose.push_back(hwnd);
      return 0;

    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;

    default:
      return DefWindowProc(hwnd, message, wParam, lParam);
  }
}

namespace Ngin {
Window::Window(uint16_t width, uint16_t height, std::string_view name, std::string_view className)
    : name(name), className(className), dimensions(width, height) {
  HINSTANCE instance = GetModuleHandle(nullptr);
  std::wstring classNameW(className.begin(), className.end());
  std::wstring windowName(name.begin(), name.end());

  logTrace("Setting wndClass desc");
  WNDCLASSEXW wc{
    .cbSize = sizeof(wc),
    .style = CS_OWNDC,
    .lpfnWndProc = WndProc,
    .cbClsExtra = 0,
    .cbWndExtra = 0,
    .hInstance = instance,
    .hIcon = LoadIcon(nullptr, IDI_APPLICATION),
    .hCursor = LoadCursor(0, IDC_ARROW),
    .hbrBackground = nullptr,
    .lpszMenuName = nullptr,
    .lpszClassName = classNameW.c_str(),
    .hIconSm = LoadIcon(nullptr, IDI_APPLICATION)
  };

  logTrace("Registering class");
  w_class = RegisterClassExW(&wc);
  if (!w_class) {
    Ngin::logError(
      std::format("Creating window failed. Couldnt RegisterClass Error code {}", GetLastError()));
    logFatal(
      std::format("Creating window failed. Couldnt RegisterClass Error code {}", GetLastError()));
    std::exit(1);
    return;
  }

  logTrace("Creating windowex");
  hwnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW, classNameW.c_str(),
    windowName.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, dimensions.x,
    dimensions.y, nullptr, nullptr, instance, nullptr);

  if (hwnd == nullptr) {
    logFatal(std::format("Creating Window failed: Couldnt CreateWindowEx Error code {}",
      GetLastError()));
    std::exit(1);
    return;
  }

  logTrace("Creating RHI");
  Error err = RHI::Create(hwnd, dimensions, fullscreen, rhi);

  if (err) {
    Ngin::logError(std::format("RHI could not be created: {}", static_cast<int>(err.code)));
    Ngin::logError(err.message);
    std::exit(1);
    return;
  }

  windows.insert_or_assign(hwnd, this);
};

Error Window::SetShow(CmdShow showType) {
  if (hwnd == 0) {
    return Error{PlatformError, "Window Handle is NULL"};
  }
  ShowWindow(hwnd, showType);
  return Error{};
}

Window* Window::GetWindow(HWND hwnd) {
  if (windows.contains(hwnd)) {
    return windows.at(hwnd);
  }
  return nullptr;
}

void Window::CheckForClosableWindows() {
  if (windowsToClose.size() <= 0) {
    return;
  }
  for (const auto windowToClose : windowsToClose) {
    if (windows.contains(windowToClose)) {
      Close(*windows.at(windowToClose));
    }
  }
}

Error Window::Update() {
  if (hwnd == 0) {
    return Error{PlatformError, "Window handle is NULL"};
  }

  CheckForClosableWindows();

  MSG msg;

  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) {
      Close();
      hwnd = 0;
      return Error{Exit, "Window was closed"};
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  Error err = rhi->BeginFrame();
  if (err)
    return err;

  rhi->Draw(dimensions);

  err = rhi->EndFrame();
  if (err)
    return err;
  err = rhi->Update();
  if (err)
    return err;
  return Error{};
}

Error Window::Resize(bool autoResize, std::optional<Vec2Rounded> newDimensions) {
  if (autoResize) {
    RECT cr;
    if (GetClientRect(hwnd, &cr)) {
      dimensions.x = cr.right - cr.left;
      dimensions.y = cr.bottom - cr.top;
    } else {
      return Error{PlatformError, "Couldnt get Client Rect for Resizing"};
    }
  } else {
    if (newDimensions)
      dimensions = newDimensions.value();
    else
      return Error{
        PlatformError, "No newDimensions were passed even though autoResize was false"
      };
  }

  rhi->ReleaseBuffers();

  Error err = rhi->Resize(dimensions.x, dimensions.y);
  if (err)
    return err;

  return rhi->GetBuffers();
}

Error Window::SetFullscreen(bool enable) {
  fullscreen = enable;

  DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  DWORD exStyle = WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW;

  if (fullscreen) {
    style = WS_POPUP | WS_VISIBLE;
    exStyle = WS_EX_APPWINDOW;
  }
  SetWindowLongW(hwnd, GWL_STYLE, style);
  SetWindowLongW(hwnd, GWL_EXSTYLE, exStyle);
  if (fullscreen) {
    HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (GetMonitorInfo(monitor, &monitorInfo)) {
      SetWindowPos(hwnd, nullptr, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
        monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
        monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top, SWP_NOZORDER);
      Resize();
    }
  } else {
    SetShow(ShowMaximized);
  }
  return Error{};
}

Error Window::Close() {
  if (hwnd) {
    DestroyWindow(hwnd);
    windows.erase(hwnd);
    hwnd = nullptr;
  }
  if (w_class) {
    std::wstring classNameW(className.begin(), className.end());
    UnregisterClassW(classNameW.c_str(), GetModuleHandle(nullptr));
  }
  return Error{};
}
Error Window::Close(Window& window) {
  if (window.hwnd) {
    DestroyWindow(window.hwnd);
    windows.erase(window.hwnd);
    window.hwnd = nullptr;
  }
  if (window.w_class) {
    std::wstring classNameW(window.className.begin(), window.className.end());
    UnregisterClassW(classNameW.c_str(), GetModuleHandle(nullptr));
  }
  return Error{};
}

}  // namespace Ngin
