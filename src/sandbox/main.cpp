#include <ngin/core/DXDebugLayer.h>
#include <ngin/core/base.h>
#include <ngin/platform/window.h>

int main() {
  DXDebugLayer::Get().Init();
  Ngin::logInfo("Sandbox starting.");
  {
    Ngin::Error err;
    auto window = Ngin::Window(1200, 720, "test", "test");

    if (err.code > Ngin::ErrorCode::None) {
      Ngin::logFatal(std::format("Window could not be created {}\n\r{}",
        static_cast<int>(err.code), err.message));
      return 1;
    }

    window.SetShow(Ngin::Window::CmdShow::ShowNormal);
    Ngin::logTrace("Sandbox started.");
    DXDebugLayer::Get().Report();
    Ngin::logTrace("Updating Sandbox");
    err = window.Update();
    Ngin::logTrace("Entering main loop");

    while (!err) {
      err = window.Update();
    }
    Ngin::logTrace("Sandbox terminating.");
    Ngin::Window::Close(window);
  }
  DXDebugLayer::Get().Shutdown();

  return 0;
}
