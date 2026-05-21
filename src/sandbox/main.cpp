#include <ngin/core/base.h>
#include <ngin/platform/window.h>
#include <synchapi.h>

int main() {
  Ngin::logInfo("Sandbox starting.");
  if (Ngin::Window::Create(Ngin::Window::Properties(1200, 720, "Test")) !=
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

  return 0;
}
