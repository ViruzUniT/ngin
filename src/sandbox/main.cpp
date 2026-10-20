#include <ngin/core/DXDebugLayer.h>
#include <ngin/core/base.h>
#include <ngin/platform/window.h>

int main() {
  DXDebugLayer::Get().Init();
  Ngin::logInfo("Sandbox starting.");
  {
    auto window = Ngin::Window::Window(1200, 720, "test", "test");
    Ngin::Error err = Ngin::Window::Create(window);

    if (err.code > Ngin::ErrorCode::None) {
      Ngin::logFatal(std::format("Window could not be created {}\n\r{}",
          static_cast<int>(err.code), err.message));
      return 1;
    }

    Ngin::Window::SetShow(window, Ngin::Window::CmdShow::ShowNormal);
    Ngin::logTrace("Sandbox started.");
    DXDebugLayer::Get().Report();
    Ngin::logTrace("Updating Sandbox");
    Ngin::Error state = Ngin::Window::Update(window);
    Ngin::logTrace("Entering main loop");

    while (state.code <= Ngin::ErrorCode::None) {
      state = Ngin::Window::Update(window);
    }
    Ngin::logTrace("Sandbox terminating.");
    Ngin::Window::Close(window);
  }
  DXDebugLayer::Get().Shutdown();

  return 0;
}
