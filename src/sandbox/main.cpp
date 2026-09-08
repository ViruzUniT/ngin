#include <ngin/core/DXDebugLayer.h>
#include <ngin/core/base.h>
#include <ngin/platform/window.h>
#include <synchapi.h>

int main() {
  DXDebugLayer::Get().Init();
  Ngin::logInfo("Sandbox starting.");
  {
    auto window = Ngin::Window::Window(1200, 720, "test", "test");
    Ngin::Error err = Ngin::Window::Create(window);
    if (err.code > Ngin::ErrorCode::None) {
      Ngin::logFatal(std::format("Window could not be created {}, {}", static_cast<int>(err.code),
          err.message));
      return 1;
    }

    Ngin::Window::SetShow(window, Ngin::Window::CmdShow::ShowNormal);
    Ngin::logInfo("Sandbox started.");
    DXDebugLayer::Get().Report();
    Ngin::logInfo("Updating Sandbox");
    Ngin::Error state = Ngin::Window::Update(window);
    Ngin::logInfo("Entering main loop");
    while (state.code <= Ngin::ErrorCode::None) {
      state = Ngin::Window::Update(window);
      // Sleep(1);
    }
    Ngin::logInfo("Sandbox terminating.");
    Ngin::Window::Close(window);
  }
  DXDebugLayer::Get().Shutdown();

  return 0;
}
