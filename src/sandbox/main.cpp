#include <ngin/core/DXDebugLayer.h>
#include <ngin/core/base.h>
#include <ngin/platform/window.h>
#include <synchapi.h>

int main() {
  DXDebugLayer::Get().Init();
  Ngin::logInfo("Sandbox starting.");
  if (Ngin::Window::Create(Ngin::Window::Properties(1200, 720, "test", "test")) !=
      Ngin::ErrorCode::None) {
    Ngin::logFatal("Window could not be created");
    return 1;
  }

  Ngin::Window::SetShow(Ngin::Window::CmdShow::ShowNormal);
  Ngin::logInfo("Sandbox started.");
  while (Ngin::Window::Update() == Ngin::ErrorCode::None) {
    // Sleep(1);
  }
  Ngin::logInfo("Sandbox terminating.");
  DXDebugLayer::Get().Shutdown();

  return 0;
}
