#include <ngin/core/DXDebugLayer.h>
#include <ngin/core/base.h>
#include <ngin/platform/window.h>
#include <synchapi.h>

int main() {
  DXDebugLayer::Get().Init();
  Ngin::logInfo("Sandbox starting.");
  {
    auto window = Ngin::Window::Window(1200, 720, "test", "test");
    Ngin::ErrorCode err = Ngin::Window::Create(window);
    if (err != Ngin::ErrorCode::None) {
      Ngin::logFatal(std::format("Window could not be created {}", static_cast<int>(err)));
      return 1;
    }

    Ngin::Window::SetShow(window, Ngin::Window::CmdShow::ShowNormal);
    Ngin::logInfo("Sandbox started.");
    while (Ngin::Window::Update(window) == Ngin::ErrorCode::None) {
      // Sleep(1);
    }
    Ngin::logInfo("Sandbox terminating.");
  }
  DXDebugLayer::Get().Shutdown();

  return 0;
}
